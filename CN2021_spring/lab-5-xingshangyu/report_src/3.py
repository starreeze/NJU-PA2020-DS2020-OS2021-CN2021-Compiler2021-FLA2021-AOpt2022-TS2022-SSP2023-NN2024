def send_arp_request(self, index):
	'''
	increase retry, remove if exceed, return new index
	'''
	entry = self.data[index]
	if entry[4] >= self.max_retry:
		# host unreachable ...
	
	# send request ...
	
	return index + 1