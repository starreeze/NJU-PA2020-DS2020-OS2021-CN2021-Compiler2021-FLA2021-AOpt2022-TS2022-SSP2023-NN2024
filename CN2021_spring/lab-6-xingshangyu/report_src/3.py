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