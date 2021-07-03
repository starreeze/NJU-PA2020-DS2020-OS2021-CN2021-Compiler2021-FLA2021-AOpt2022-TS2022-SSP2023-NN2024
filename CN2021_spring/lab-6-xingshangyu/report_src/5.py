def check_timeout(self):
	if time() - self.timestamp >= self.to:
		self.timeouts += 1
		for entry in self.packets:
			if not entry[1]:
				seqnum = int.from_bytes(entry[0][3].to_bytes()[:4], 'big')
				log_info(f'timeout resend: seq = {seqnum}')
				self.net.send_packet(self.net.interfaces()[0], entry[0])
				self.retransmit += 1
		self.timestamp = time()