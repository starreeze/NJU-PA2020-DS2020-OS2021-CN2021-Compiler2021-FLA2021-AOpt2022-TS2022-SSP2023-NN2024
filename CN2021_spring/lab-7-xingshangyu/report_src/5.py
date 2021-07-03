if path in self.cacheTable and not self.cacheTable.expired(path):
	return self.cacheTable.getHeaders(path), self.cacheTable.getBody(path)
response = self.requestMainServer(path)
if response is None:
	return None
headers, body = response.getheaders(), response.read()
self.cacheTable.setHeaders(path, headers)
self.cacheTable.appendBody(path, body)
return headers, body