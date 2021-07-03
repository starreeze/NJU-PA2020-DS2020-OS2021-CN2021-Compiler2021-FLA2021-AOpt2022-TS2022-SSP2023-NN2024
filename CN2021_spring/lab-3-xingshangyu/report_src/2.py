class ArpTable:
    data = {} # ip -> (mac, timestamp)
    timeout = 10

    def update(self, ip, mac):
        self.data[ip] = (mac, time())

    def get(self, mac):
        t = self.data.get(mac)
        if t == None or time() - t[1] >= self.timeout:
            return None
        return t[0]

    def __str__(self) -> str:
        res = ''
        for key in self.data:
            if time() - self.data[key][1] < self.timeout:
                res += f"'{key.compressed}': '{self.data[key][0].toStr()}', "
        return res