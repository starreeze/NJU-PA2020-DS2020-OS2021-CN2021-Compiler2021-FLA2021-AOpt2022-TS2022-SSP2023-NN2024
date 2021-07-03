class ForwardTable:
    data = {} # mac -> intf

    def update_in(self, mac, intf):
        self.data[mac] = intf

    def update_out(self, mac, intf):
        pass

    def get(self, mac):
        return self.data.get(mac)