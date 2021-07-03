class ForwardTable:
    data = {} # mac -> (intf, timestamp)
    timeout = 10

    def update_in(self, mac, intf):
        self.data[mac] = (intf, time())

    def update_out(self, max, intf):
        pass

    def get(self, mac):
        t = self.data.get(mac)
        if t == None or time() - t[1] >= self.timeout:
            return None
        return t[0]