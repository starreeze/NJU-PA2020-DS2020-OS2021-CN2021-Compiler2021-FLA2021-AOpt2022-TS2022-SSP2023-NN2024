#!/usr/bin/env python3

from time import time
import switchyard
from switchyard.lib.address import *
from switchyard.lib.packet import *
from switchyard.lib.userlib import *
from collections import deque

blaster_mac = '10:00:00:00:00:01'
blastee_mac = '20:00:00:00:00:01'
blaster_intf_mac = '40:00:00:00:00:01'
blastee_intf_mac = '40:00:00:00:00:02'


class Buffer:
    packets = deque() # [packet, ack_state]
    seqnum = 0
    retransmit = 0
    timeouts = 0

    def __init__(self, net, blasteeIp, size, num, to, pkt_len: int) -> None:
        self.net = net
        self.blasteeIp = blasteeIp
        self.num = num
        self.to = to
        self.pkt_len = pkt_len
        self.timestamp = time()
        self.start_time = self.timestamp
        # send min(size, num) packets
        for i in range(size):
            packet = self.new_packet()
            if packet is not None:
                log_info(f'init send: {self.seqnum - 1}')
                net.send_packet(net.interfaces()[0], packet)
                self.packets.append([packet, False])

    def new_packet(self):
        if self.seqnum >= self.num:
            return None
        pkt = Ethernet() + IPv4() + UDP()
        pkt[1].protocol = IPProtocol.UDP
        pkt[0].src = blaster_mac
        pkt[0].dst = blaster_intf_mac
        pkt[1].src = self.net.interfaces()[0].ipaddr
        pkt[1].dst = self.blasteeIp
        pkt += RawPacketContents(self.seqnum.to_bytes(4, 'big') + self.pkt_len.to_bytes(2, 'big') + (0).to_bytes(self.pkt_len, 'big'))
        self.seqnum += 1
        return pkt

    def ack(self, seqnum):
        for entry in self.packets:
            if entry[0][3].to_bytes()[:4] == seqnum:
                entry[1] = True
                break
        while len(self.packets) and self.packets[0][1]:
            self.packets.popleft()
            packet = self.new_packet()
            if packet is not None:
                self.packets.append([packet, False])
                log_info(f'ack send {self.seqnum - 1}')
                self.net.send_packet(self.net.interfaces()[0], packet)
            self.timestamp = time()
        if len(self.packets) == 0:
            self.print_info()
            self.net.shutdown()

    def check_timeout(self):
        if time() - self.timestamp >= self.to:
            self.timeouts += 1
            for entry in self.packets:
                if not entry[1]:
                    seqnum = int.from_bytes(entry[0][3].to_bytes()[:4], 'big')
                    log_info(f'timeout resend: seq = {seqnum}')
                    self.net.send_packet(self.net.interfaces()[0], entry[0])
                    self.retransmit += 1
            self.timestamp = time()

    def print_info(self):
        transmit_time = time() - self.start_time
        log_info(f'Total TX time: {transmit_time}')
        log_info(f'Number of reTX: {self.retransmit}')
        log_info(f'Number of coarse TOs: {self.timeouts}')
        log_info(f'Throughput (Bps): {(self.num + self.retransmit) * self.pkt_len / transmit_time}')
        log_info(f'Goodput (Bps): {self.num * self.pkt_len / transmit_time}')


class Blaster:
    def __init__(
            self,
            net: switchyard.llnetbase.LLNetBase,
            blasteeIp,
            num,
            length="100",
            senderWindow="5",
            timeout="300",
            recvTimeout="100"
    ):
        self.net = net
        self.rto = int(recvTimeout) / 1000
        self.buf = Buffer(net, blasteeIp, int(senderWindow), int(num), int(timeout) / 1000, int(length))

    def handle_packet(self, recv: switchyard.llnetbase.ReceivedPacket):
        _, fromIface, packet = recv
        seqnum = packet[3].to_bytes()[:4]
        log_info(f"I got a packet: seq = {int.from_bytes(seqnum, 'big')}")
        self.buf.ack(seqnum)

    def handle_no_packet(self):
        log_info("Didn't receive anything")
        self.buf.check_timeout()

    def start(self):
        '''A running daemon of the blaster.
        Receive packets until the end of time.
        '''
        while True:
            try:
                recv = self.net.recv_packet(timeout=self.rto)
            except NoPackets:
                self.handle_no_packet()
                continue
            except Shutdown:
                break

            self.handle_packet(recv)

        self.shutdown()

    def shutdown(self):
        self.net.shutdown()


def main(net, **kwargs):
    blaster = Blaster(net, **kwargs)
    blaster.start()
