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
		else: # create Ethernet header and send out
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