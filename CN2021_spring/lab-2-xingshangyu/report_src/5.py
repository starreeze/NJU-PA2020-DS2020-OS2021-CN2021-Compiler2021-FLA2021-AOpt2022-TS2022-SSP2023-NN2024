class ForwardTable:
    data = {} # mac -> (intf, age)
    max_size = 5

    def update_in(self, mac, intf):
        for key in self.data.keys():
            self.data[key] = (self.data[key][0], self.data[key][1] + 1)
        if self.data.get(mac) != None:
            self.data[mac] = (intf, self.data[mac][1])
        else:
            self.data[mac] = (intf, 0)
            if len(self.data) > self.max_size:
                del self.data[max(self.data, key=lambda x: self.data[x][1])]

    def update_out(self, mac, intf):
        if self.data.get(mac) != None:
            self.data[mac] = (self.data[mac][0], 0)

    def get(self, mac):
        t = self.data.get(mac)
        if t == None:
            return None
        return t[0]