res = self.server.touchItem(self.path)
if res is None:
    self.send_error(HTTPStatus.NOT_FOUND, "'File not found'")
    return
headers, body = res
self.sendHeaders(headers)
self.sendBody(body)