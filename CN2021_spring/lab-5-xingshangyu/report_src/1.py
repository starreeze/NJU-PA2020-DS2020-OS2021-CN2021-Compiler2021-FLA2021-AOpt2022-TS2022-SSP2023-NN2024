def handle_packet(self, recv: switchyard.llnetbase.ReceivedPacket):
    timestamp, ifaceName, packet = recv
    # arp reply ...

    # forward & icmp handle
    hip_idx = packet.get_header_index(IPv4)
    if(hip_idx != -1):
        hip = packet[hip_idx]        
        try:
            self.net.interface_by_ipaddr(hip.dst)
            # target self
            hicmp_idx = packet.get_header_index(ICMP)
            if hicmp_idx != -1 and packet[hicmp_idx].icmptype == ICMPType.EchoRequest:
                # ping reply ...
                
            else:
                # check ttl ...

                # destination port unreachable ...
                
        except KeyError: # not self
            # check ttl ...

            self.forward_packet(packet, ifaceName)