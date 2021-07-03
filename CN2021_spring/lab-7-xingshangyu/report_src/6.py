def touchItem(self, path: str):
	assert(self.response is None)
	if path in self.cacheTable and not self.cacheTable.expired(path):
		return self.cacheTable.getHeaders(path), self.cacheTable.getBody(path)
	response = self.requestMainServer(path)
	if response is None:
		return None
	headers = response.getheaders()
	self.cacheTable.setHeaders(path, headers)
	self.response = response
	self.path = path
	return headers, None # to be continue in get_body

def get_body(self):
	if self.response is None:
		raise StopIteration()
	length = self.response.readinto(self.buffer)
	if length < BUFFER_SIZE:
		self.response = None
	self.cacheTable.appendBody(self.path, self.buffer[:length])
	return self.buffer[:length]