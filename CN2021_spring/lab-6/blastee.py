#!/usr/bin/env python3

from struct import pack
import switchyard
from switchyard.lib.address import *
from switchyard.lib.packet import *
from switchyard.lib.userlib import *

blaster_mac = "10:00:00:00:00:01"
blastee_mac = "20:00:00:00:00:01"
blaster_intf_mac = "40:00:00:00:00:01"
blastee_intf_mac = "40:00:00:00:00:02"


class Blastee:
    def __init__(self, net: switchyard.llnetbase.LLNetBase, blasterIp, num):
        self.net = net
        self.blasterIp = blasterIp
        self.num = int(num)

    def handle_packet(self, recv: switchyard.llnetbase.ReceivedPacket):
        _, fromIface, packet = recv
        log_info(f"I got a packet: {packet}")
        reply = (
            Ethernet(src=blastee_mac, dst=blastee_intf_mac, ethertype=EtherType.IP)
            + IPv4(
                protocol=IPProtocol.UDP,
                src=self.net.interfaces()[0].ipaddr,
                dst=self.blasterIp,
            )
            + UDP()
        )
        payload_len = int.from_bytes(packet[3].to_bytes()[4:6], "big")
        if payload_len >= 8:
            reply += packet[3].to_bytes()[:4] + packet[3].to_bytes()[6:6+8]
        else:
            reply += packet[3].to_bytes()[:4] + packet[3].to_bytes()[6:] + (0).to_bytes(8 - payload_len, "big")
        assert(len(reply[3]) == 12)
        log_info(f"send: {reply}")
        self.net.send_packet(fromIface, reply)
        seqnum = int.from_bytes(packet[3].to_bytes()[:4], "big")
        # if seqnum == self.num - 1:
        #     self.net.shutdown()

    def start(self):
        """A running daemon of the blastee.
        Receive packets until the end of time.
        """
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
    blastee = Blastee(net, **kwargs)
    blastee.start()
