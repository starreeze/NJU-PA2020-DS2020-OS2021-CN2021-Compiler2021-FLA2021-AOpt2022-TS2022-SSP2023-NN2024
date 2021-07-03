def query(self, address: IPv4Address):
    # return [next hop address, interface] or None
    for entry in self.data:
        if address in entry[0]:
            return entry[1:]
    return None