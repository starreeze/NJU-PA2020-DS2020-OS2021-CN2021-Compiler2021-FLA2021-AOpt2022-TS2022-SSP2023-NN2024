#!/usr/bin/env python3

'''
Basic IPv4 router (static routing) in Python.
'''

from ipaddress import ip_interface
from time import time
import switchyard
from switchyard.lib.userlib import *
from switchyard.lib.address import *


class ArpTable(object):
    data = {} # ip -> (mac, timestamp)
    timeout = 1e9

    def update(self, ip, mac):
        self.data[ip] = (mac, time())

    def query(self, ip):
        t = self.data.get(ip)
        if t == None or time() - t[1] >= self.timeout:
            return None
        return t[0]

    def __str__(self) -> str:
        res = ''
        for key in self.data:
            if time() - self.data[key][1] < self.timeout:
                res += f"'{key.compressed}': '{self.data[key][0].toStr()}', "
        return res


class ForwardTable(object):
    data = [] # [match prefix, next hop address, interface]

    def __init__(self, interfaces: list) -> None:
        for intf in interfaces:
            self.data.append([ip_interface(str(intf.ipaddr) + '/' + str(intf.netmask)).network, IPv4Address('0.0.0.0'), intf.name])
        with open('forwarding_table.txt', 'r') as fp:
            lines = fp.readlines()
        # [network address, subnet address, next hop address, interface]
        for line in lines:
            strs = line.split(' ')
            if strs[3][-1] == '\n':
                strs[3] = strs[3][:-1]
            self.data.append([IPv4Network(strs[0] + '/' + strs[1]), IPv4Address(strs[2]), strs[3]])
        # sort by length: longest match
        self.data.sort(key=lambda entry: int(entry[0].netmask), reverse=True)
        
    def query(self, address: IPv4Address):
        # return [next hop address, interface] or None
        for entry in self.data:
            if address in entry[0]:
                return entry[1:]
        return None


class ArpQueue(object): # packets waiting for arp reply
    data = [] # [packet, intf, next_ip, timestamp, retries, ifaceName]
    unreach = set()
    max_retry = 5
    timeout = 1.0

    def __init__(self, net) -> None:
        self.net = net

    def send_arp_request(self, index):
        # increase retry, remove if exceed, return new index
        entry = self.data[index]
        if entry[4] >= self.max_retry:
            # host unreachable
            hip = entry[0].get_header(IPv4)
            if not (str(hip.src), str(hip.dst)) in self.unreach:
                self.unreach.add((str(hip.src), str(hip.dst)))
                reply = create_icmp(ICMPType.DestinationUnreachable, 1, entry[0], self.net.interface_by_name(entry[5]).ipaddr)
                rhicmp_idx = reply.get_header_index(ICMP)
                reply[rhicmp_idx].icmpdata.data = packet_data(entry[0])
                reply[rhicmp_idx].icmpdata.origdgramlen = len(entry[0])
                global router
                router.forward_packet(reply, "")
            del self.data[index]
            return index
        entry[3] = time()
        entry[4] += 1
        # send request
        ether = Ethernet()
        ether.src = entry[1].ethaddr
        ether.dst = 'ff:ff:ff:ff:ff:ff'
        ether.ethertype = EtherType.ARP
        arp = Arp(operation=ArpOperation.Request,
                senderhwaddr=entry[1].ethaddr,
                senderprotoaddr=entry[1].ipaddr,
                targethwaddr='ff:ff:ff:ff:ff:ff',
                targetprotoaddr=entry[2])
        arppacket = ether + arp
        self.net.send_packet(entry[1], arppacket)
        return index + 1

    def insert(self, packet, intf, next_ip, ifaceName):
        log_info(f"{packet} waiting for arp reply")
        # insert a timeout entry to be retried
        self.data.append([packet, intf, next_ip, -2 * self.timeout, 0, ifaceName])

    def release(self, reply):
        # send out packet and remove entry if arp reply matches
        ip = reply.senderprotoaddr
        mac = reply.senderhwaddr
        i = 0
        while i < len(self.data):
            packet = self.data[i][0]
            if self.data[i][2] == ip:
                intf = self.data[i][1]
                # forward packet without updating arp table which is done in caller stage
                log_info(f"{packet} released from {intf}")
                hip_idx = packet.get_header_index(Ethernet)
                packet[hip_idx].src = intf.ethaddr
                packet[hip_idx].dst = mac
                self.net.send_packet(intf, packet)
                del self.data[i]
            else:
                i += 1

    def check_timeout(self):
        i = 0
        while i < len(self.data):
            if time() - self.data[i][3] >= self.timeout:
                log_info(f"{self.data[i][0]} arp retry")
                i = self.send_arp_request(i)
            else:
                i += 1


def create_icmp(type, code, packet, srcip):
    # create ip
    hip = IPv4()
    hip.src = srcip
    hip.dst = packet.get_header(IPv4).src
    hip.ttl = 100
    hip.protocol = IPProtocol.ICMP
    # create icmp
    hicmp = ICMP()
    hicmp.icmptype = type
    hicmp.icmpcode = code
    return Ethernet() + hip + hicmp


def packet_data(packet):
    idx = packet.get_header_index(Ethernet)
    if idx != -1:
        del packet[idx]
    return packet.to_bytes()[:28]


class Router(object):
    def __init__(self, net: switchyard.llnetbase.LLNetBase):
        self.net = net
        # other initialization stuff here
        self.arpTable = ArpTable()
        self.forwardTable = ForwardTable(net.interfaces())
        self.arpQueue = ArpQueue(net)

    def handle_packet(self, recv: switchyard.llnetbase.ReceivedPacket):
        timestamp, ifaceName, packet = recv
        # arp reply
        harp = packet.get_header(Arp)
        if harp is not None:
            self.arpTable.update(harp.senderprotoaddr, harp.senderhwaddr)
            log_info(str(self.arpTable))
            if harp.operation == ArpOperation.Request:
                for intf in self.net.interfaces():
                    if intf.ipaddr == harp.targetprotoaddr:
                        response = create_ip_arp_reply(intf.ethaddr, harp.senderhwaddr, intf.ipaddr, harp.senderprotoaddr)
                        self.net.send_packet(ifaceName, response)
            elif harp.operation == ArpOperation.Reply:
                self.arpQueue.release(harp)
        # forward & icmp handle
        hip_idx = packet.get_header_index(IPv4)
        if(hip_idx != -1):
            hip = packet[hip_idx]
            packet[hip_idx].ttl -= 1
            try:
                self.net.interface_by_ipaddr(hip.dst)
                # target self
                hicmp_idx = packet.get_header_index(ICMP)
                if hicmp_idx != -1 and packet[hicmp_idx].icmptype == ICMPType.EchoRequest:
                    # ping reply
                    reply = create_icmp(ICMPType.EchoReply, 0, packet, self.net.interface_by_name(ifaceName).ipaddr)
                    rhicmp_idx = reply.get_header_index(ICMP)
                    reply[rhicmp_idx].icmpdata.data = packet[hicmp_idx].icmpdata.data
                    reply[rhicmp_idx].icmpdata.identifier = packet[hicmp_idx].icmpdata.identifier
                    reply[rhicmp_idx].icmpdata.sequence = packet[hicmp_idx].icmpdata.sequence
                    self.forward_packet(reply, "")
                else:
                    # check ttl
                    if packet[hip_idx].ttl == 0:
                        reply = create_icmp(ICMPType.TimeExceeded, 0, packet, self.net.interface_by_name(ifaceName).ipaddr)
                        rhicmp_idx = reply.get_header_index(ICMP)
                        reply[rhicmp_idx].icmpdata.data = packet_data(packet)
                        reply[rhicmp_idx].icmpdata.origdgramlen = len(packet)
                        self.forward_packet(reply, "")
                        return
                    # destination port unreachable
                    reply = create_icmp(ICMPType.DestinationUnreachable, 3, packet, self.net.interface_by_name(ifaceName).ipaddr)
                    rhicmp_idx = reply.get_header_index(ICMP)
                    reply[rhicmp_idx].icmpdata.data = packet_data(packet)
                    reply[rhicmp_idx].icmpdata.origdgramlen = len(packet)
                    self.forward_packet(reply, "")
            except KeyError:
                # not self
                # check ttl
                if packet[hip_idx].ttl == 0:
                    reply = create_icmp(ICMPType.TimeExceeded, 0, packet, self.net.interface_by_name(ifaceName).ipaddr)
                    rhicmp_idx = reply.get_header_index(ICMP)
                    reply[rhicmp_idx].icmpdata.data = packet_data(packet)
                    reply[rhicmp_idx].icmpdata.origdgramlen = len(packet)
                    self.forward_packet(reply, "")
                    return
                self.forward_packet(packet, ifaceName)
    
    def forward_packet(self, packet, ifaceName):
        hip = packet.get_header(IPv4)
        ip_intf = self.forwardTable.query(hip.dst)
        if ip_intf == None:
            # destination network unreachable
            assert(ifaceName) # must not be an icmp error message just generated
            reply = create_icmp(ICMPType.DestinationUnreachable, 0, packet, self.net.interface_by_name(ifaceName).ipaddr)
            rhicmp_idx = reply.get_header_index(ICMP)
            reply[rhicmp_idx].icmpdata.data = packet_data(packet)
            reply[rhicmp_idx].icmpdata.origdgramlen = len(packet)
            packet = reply
            hip = packet.get_header(IPv4)
            ip_intf = self.forwardTable.query(hip.dst)
        next_ip, next_intf = tuple(ip_intf)
        intf = self.net.interface_by_name(next_intf)
        if intf.ipaddr != next_ip:
            # create Ethernet header and send out
            if next_ip == IPv4Address('0.0.0.0'):
                next_ip = hip.dst
            mac = self.arpTable.query(next_ip)
            if mac == None:
                # construct arp request
                self.arpQueue.insert(packet, intf, next_ip, ifaceName)
            else: # forward directly
                eth_idx = packet.get_header_index(Ethernet)
                packet[eth_idx].src = intf.ethaddr
                packet[eth_idx].dst = mac
                self.net.send_packet(intf, packet)
                log_info(f"directly forward {packet} out {intf}")

    def start(self):
        '''A running daemon of the router.
        Receive packets until the end of time.
        '''
        while True:
            self.arpQueue.check_timeout()
            try:
                recv = self.net.recv_packet(timeout=1.0)
            except NoPackets:
                continue
            except Shutdown:
                break

            self.handle_packet(recv)
        self.stop()

    def stop(self):
        self.net.shutdown()

router: Router


def main(net):
    '''
    Main entry point for router.  Just create Router
    object and get it going.
    '''
    global router
    router = Router(net)
    router.start()
