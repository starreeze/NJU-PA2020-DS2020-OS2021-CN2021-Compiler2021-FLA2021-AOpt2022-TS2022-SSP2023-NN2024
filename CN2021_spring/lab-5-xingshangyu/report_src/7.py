reply = create_icmp(ICMPType.EchoReply, 0, packet, self.net.interface_by_name(ifaceName).ipaddr)
rhicmp_idx = reply.get_header_index(ICMP)
reply[rhicmp_idx].icmpdata.data = packet[hicmp_idx].icmpdata.data
reply[rhicmp_idx].icmpdata.identifier = packet[hicmp_idx].icmpdata.identifier
reply[rhicmp_idx].icmpdata.sequence = packet[hicmp_idx].icmpdata.sequence
self.forward_packet(reply, "")