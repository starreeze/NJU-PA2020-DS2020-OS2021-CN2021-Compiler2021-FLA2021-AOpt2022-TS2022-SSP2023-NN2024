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
        if seqnum == self.num - 1:
            self.net.shutdown()