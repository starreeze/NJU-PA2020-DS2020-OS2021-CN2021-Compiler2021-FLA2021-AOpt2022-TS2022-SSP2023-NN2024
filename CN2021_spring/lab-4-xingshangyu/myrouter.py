#!/usr/bin/env python3

'''
Basic IPv4 router (static routing) in Python.
'''

from ipaddress import ip_interface
from time import sleep, time
import switchyard
from switchyard.lib.userlib import *
from switchyard.lib.address import *
import threading


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
    data = [] # [packet, intf, next_ip, timestamp, retries]
    max_retry = 5
    timeout = 1.0

    def __init__(self, net) -> None:
        self.net = net

    def send_arp_request(self, index):
        # increase retry, remove if exceed, return new index
        entry = self.data[index]
        if entry[4] >= self.max_retry:
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

    def insert(self, packet, intf, next_ip):
        log_info(f"{packet} waiting for arp reply")
        # insert a timeout entry to be retried
        self.data.append([packet, intf, next_ip, -2 * self.timeout, 0])

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


class ArpThread(threading.Thread):
    def __init__(self, queue: ArpQueue):
        threading.Thread.__init__(self, daemon=True)
        self.arpQueue = queue
    
    def run(self):
        while True:
            self.arpQueue.check_timeout()
            sleep(0.1)


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
        # forward packet
        hip_idx = packet.get_header_index(IPv4)
        if(hip_idx != -1):
            packet[hip_idx].ttl -= 1 # modify in the future
            hip = packet[hip_idx]
            ip_intf = self.forwardTable.query(hip.dst)
            if ip_intf == None:
                pass # modify in the future
            else:
                next_ip, next_intf = tuple(ip_intf)
                intf = self.net.interface_by_name(next_intf)
                if intf.ipaddr == next_ip:
                    pass # modify in the future
                else:
                    # create Ethernet header and send out
                    if next_ip == IPv4Address('0.0.0.0'):
                        next_ip = hip.dst
                    mac = self.arpTable.query(next_ip)
                    if mac == None:
                        # construct arp request
                        self.arpQueue.insert(packet, intf, next_ip)
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
        arpThread = ArpThread(self.arpQueue)
        arpThread.start()
        while True:
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


def main(net):
    '''
    Main entry point for router.  Just create Router
    object and get it going.
    '''
    router = Router(net)
    router.start()
