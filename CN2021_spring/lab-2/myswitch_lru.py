'''
Ethernet learning switch in Python.

Note that this file currently has the code to implement a "hub"
in it, not a learning switch.  (I.e., it's currently a switch
that doesn't learn.)
'''
import switchyard
from switchyard.lib.userlib import *


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


def main(net: switchyard.llnetbase.LLNetBase):
    my_interfaces = net.interfaces()
    mymacs = [intf.ethaddr for intf in my_interfaces]

    table = ForwardTable()

    while True:
        try:
            _, fromIface, packet = net.recv_packet()
        except NoPackets:
            continue
        except Shutdown:
            break
        
        log_debug (f"In {net.name} received packet {packet} on {fromIface}")
        eth = packet.get_header(Ethernet)

        table.update_in(eth.src, fromIface)

        if eth is None:
            log_info("Received a non-Ethernet packet?!")
            return
        if eth.dst in mymacs:
            log_info("Received a packet intended for me")
        
        else:
            dst_intf = table.get(eth.dst)
            if dst_intf != None and eth.dst != 'ff:ff:ff:ff:ff:ff':
                net.send_packet(dst_intf, packet)
                table.update_out(eth.dst, dst_intf)
                log_info(f"Sending packet {packet} to {dst_intf}")
            else:
                for intf in my_interfaces:
                    if fromIface!= intf.name:
                        log_info(f"Flooding packet {packet} to {intf.name}")
                        net.send_packet(intf, packet)

    net.shutdown()
