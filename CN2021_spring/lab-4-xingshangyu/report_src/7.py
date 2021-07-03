def check_timeout(self):
	i = 0
	while i < len(self.data):
		if time() - self.data[i][3] >= self.timeout:
			log_info(f"{self.data[i][0]} arp retry")
			i = self.send_arp_request(i)
		else:
			i += 1