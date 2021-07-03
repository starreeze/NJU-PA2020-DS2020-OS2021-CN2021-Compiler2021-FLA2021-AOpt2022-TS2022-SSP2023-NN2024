def handle_packet(self, recv: switchyard.llnetbase.ReceivedPacket):
        _, fromIface, packet = recv
        if packet.get_header_index(Arp) != -1:
            log_info('\033[1;33mreceive an arp?!\033[0m') # ]]
            return
        if fromIface == "middlebox-eth0":
            log_info(f"Received from blaster: {packet}")
            if random() > self.dropRate:  # not drop
                heth_idx = packet.get_header_index(Ethernet)
                packet[heth_idx].src = blastee_intf_mac
                packet[heth_idx].dst = blastee_mac
                log_info(f'send to blastee: {packet}')
                self.net.send_packet("middlebox-eth1", packet)
            else:
                log_info('\033[1;31mdrop!\033[0m') # ]]
        elif fromIface == "middlebox-eth1":
            log_info(f"Received from blastee: {packet}")
            heth_idx = packet.get_header_index(Ethernet)
            packet[heth_idx].src = blaster_intf_mac
            packet[heth_idx].dst = blaster_mac
            log_info(f'send to blaster: {packet}')
            self.net.send_packet("middlebox-eth0", packet)