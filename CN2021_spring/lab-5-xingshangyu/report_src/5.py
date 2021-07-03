# an example: host unreachable
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