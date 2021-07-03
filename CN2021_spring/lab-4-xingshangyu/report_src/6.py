def release(self, reply):
	# send out packet and remove entry if arp reply matches
	ip = reply.senderprotoaddr
	mac = reply.senderhwaddr
	i = 0
	while i < len(self.data):
		packet = self.data[i][0]
		if self.data[i][2] == ip:
			intf = self.data[i][1]
			# forward packet without updating arp table which is done in caller stage
			log_info(f"{packet} released from {intf}")
			hip_idx = packet.get_header_index(Ethernet)
			packet[hip_idx].src = intf.ethaddr
			packet[hip_idx].dst = mac
			self.net.send_packet(intf, packet)
			del self.data[i]
		i += 1