#!/usr/bin/env python3

from random import random, seed
import time

import switchyard
from switchyard.lib.address import *
from switchyard.lib.packet import *
from switchyard.lib.userlib import *

blaster_mac = '10:00:00:00:00:01'
blastee_mac = '20:00:00:00:00:01'
blaster_intf_mac = '40:00:00:00:00:01'
blastee_intf_mac = '40:00:00:00:00:02'


class Middlebox:
    def __init__(
            self,
            net: switchyard.llnetbase.LLNetBase,
            dropRate="0.19"
    ):
        self.net = net
        self.dropRate = float(dropRate)
        seed(0)  # to make result reproduceable

    def handle_packet(self, recv: switchyard.llnetbase.ReceivedPacket):
        _, fromIface, packet = recv
        if packet.get_header_index(Arp) != -1:
            log_info('\033[1;33mreceive an arp?!\033[0m') # ]]
            return
        if fromIface == "middlebox-eth0":
            log_info(f"Received from blaster: {packet}")
            '''
            Received data packet
            Should I drop it?
            If not, modify headers & send to blastee
            '''
            if random() > self.dropRate:  # not drop
                heth_idx = packet.get_header_index(Ethernet)
                packet[heth_idx].src = blastee_intf_mac
                packet[heth_idx].dst = blastee_mac
                log_info(f'send to blastee: {packet}')
                self.net.send_packet("middlebox-eth1", packet)
            else:
                log_info('\033[1;31mdrop!\033[0m') # ]]
        elif fromIface == "middlebox-eth1":
            log_info(f"Received from blastee: {packet}")
            '''
            Received ACK
            Modify headers & send to blaster. Not dropping ACK packets!
            net.send_packet("middlebox-eth0", pkt)
            '''
            heth_idx = packet.get_header_index(Ethernet)
            packet[heth_idx].src = blaster_intf_mac
            packet[heth_idx].dst = blaster_mac
            log_info(f'send to blaster: {packet}')
            self.net.send_packet("middlebox-eth0", packet)
        else:
            log_info("Oops :))")

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

        self.shutdown()

    def shutdown(self):
        self.net.shutdown()


def main(net, **kwargs):
    middlebox = Middlebox(net, kwargs['dropRate'])
    middlebox.start()
