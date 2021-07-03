def handle_packet(self, recv: switchyard.llnetbase.ReceivedPacket):
    timestamp, ifaceName, packet = recv
    # TODO: your logic here
    harp = packet.get_header(Arp)
    if harp is not None:
        for intf in self.net.interfaces():
            if intf.ipaddr == harp.targetprotoaddr:
                response = create_ip_arp_reply(intf.ethaddr, harp.senderhwaddr, intf.ipaddr, harp.senderprotoaddr)
                self.net.send_packet(ifaceName, response)