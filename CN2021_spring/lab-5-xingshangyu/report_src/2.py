def forward_packet(self, packet, ifaceName):
    hip = packet.get_header(IPv4)
    ip_intf = self.forwardTable.query(hip.dst)
    if ip_intf == None:
        # destination network unreachable ...
        
    next_ip, next_intf = tuple(ip_intf)
    intf = self.net.interface_by_name(next_intf)
    if intf.ipaddr != next_ip:
        # create Ethernet header and send out
        if next_ip == IPv4Address('0.0.0.0'):
            next_ip = hip.dst
        mac = self.arpTable.query(next_ip)
        if mac == None:
            # construct arp request ...
            
        else:
            # forward directly ...