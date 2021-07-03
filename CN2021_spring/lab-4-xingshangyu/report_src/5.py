def insert(self, packet, intf, next_ip):
	log_info(f"{packet} waiting for arp reply")
	# insert a timeout entry to be retried
	self.data.append([packet, intf, next_ip, -2 * self.timeout, 0])