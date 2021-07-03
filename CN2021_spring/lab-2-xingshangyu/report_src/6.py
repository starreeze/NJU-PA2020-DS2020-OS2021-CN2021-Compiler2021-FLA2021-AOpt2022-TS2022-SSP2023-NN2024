class ForwardTable:
    data = {} # mac -> (intf, traffic)
    max_size = 5

    def update_in(self, mac, intf):
        if self.data.get(mac) != None:
            self.data[mac] = (intf, self.data[mac][1])
        else:
            if len(self.data) >= self.max_size:
                del self.data[min(self.data, key=lambda x: self.data[x][1])]
            self.data[mac] = (intf, 0)

    def update_out(self, mac, intf):
        if self.data.get(mac) != None:
            self.data[mac] = (intf, self.data[mac][1] + 1)
    
    def get(self, mac):
        t = self.data.get(mac)
        if t == None:
            return None
        return t[0]