def send_arp_request(self, index):
	# increase retry, remove if exceed, return new index
	entry = self.data[index]
	if entry[4] >= self.max_retry:
		del self.data[index]
		return index
	entry[3] = time()
	entry[4] += 1
	# send request
	ether = Ethernet()
	ether.src = entry[1].ethaddr
	ether.dst = 'ff:ff:ff:ff:ff:ff'
	ether.ethertype = EtherType.ARP
	arp = Arp(operation=ArpOperation.Request,
			senderhwaddr=entry[1].ethaddr,
			senderprotoaddr=entry[1].ipaddr,
			targethwaddr='ff:ff:ff:ff:ff:ff',
			targetprotoaddr=entry[2])
	arppacket = ether + arp
	self.net.send_packet(entry[1], arppacket)
	return index + 1