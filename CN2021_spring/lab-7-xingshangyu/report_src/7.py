res = self.server.touchItem(self.path)
if res is None:
	self.send_error(HTTPStatus.NOT_FOUND, "'File not found'")
	return
headers, body = res
self.sendHeaders(headers)
if body is None:
	try:
		while True:
			self.sendBody(self.server.get_body())
	except StopIteration:
		pass
else:
	self.sendBody(body)