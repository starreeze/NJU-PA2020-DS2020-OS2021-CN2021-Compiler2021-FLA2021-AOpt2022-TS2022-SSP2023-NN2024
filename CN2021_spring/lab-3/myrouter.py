#!/usr/bin/env python3

'''
Basic IPv4 router (static routing) in Python.
'''

from time import time
import switchyard
from switchyard.lib.userlib import *


class ArpTable:
    data = {} # ip -> (mac, timestamp)
    timeout = 10

    def update(self, ip, mac):
        self.data[ip] = (mac, time())

    def get(self, mac):
        t = self.data.get(mac)
        if t == None or time() - t[1] >= self.timeout:
            return None
        return t[0]

    def __str__(self) -> str:
        res = ''
        for key in self.data:
            if time() - self.data[key][1] < self.timeout:
                res += f"'{key.compressed}': '{self.data[key][0].toStr()}', "
        return res


class Router(object):
    def __init__(self, net: switchyard.llnetbase.LLNetBase):
        self.net = net
        # other initialization stuff here
        self.arpTable = ArpTable()

    def handle_packet(self, recv: switchyard.llnetbase.ReceivedPacket):
        timestamp, ifaceName, packet = recv
        # TODO: your logic here
        harp = packet.get_header(Arp)
        if harp is not None:
            for intf in self.net.interfaces():
                if intf.ipaddr == harp.targetprotoaddr:
                    response = create_ip_arp_reply(intf.ethaddr, harp.senderhwaddr, intf.ipaddr, harp.senderprotoaddr)
                    self.net.send_packet(ifaceName, response)
            self.arpTable.update(harp.senderprotoaddr, harp.senderhwaddr)
            log_info(str(self.arpTable))

    def start(self):
        '''A running daemon of the router.
        Receive packets until the end of time.
        '''
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
