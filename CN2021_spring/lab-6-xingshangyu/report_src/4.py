def ack(self, seqnum):
    for entry in self.packets:
        if entry[0][3].to_bytes()[:4] == seqnum:
            entry[1] = True
            break
    while len(self.packets) and self.packets[0][1]:
        self.packets.popleft()
        packet = self.new_packet()
        if packet is not None:
            self.packets.append([packet, False])
            log_info(f'ack send {self.seqnum - 1}')
            self.net.send_packet(self.net.interfaces()[0], packet)
        self.timestamp = time()
    if len(self.packets) == 0:
        self.print_info()
        self.net.shutdown()