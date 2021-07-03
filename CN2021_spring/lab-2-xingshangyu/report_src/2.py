def main(net: switchyard.llnetbase.LLNetBase):
    my_interfaces = net.interfaces()
    mymacs = [intf.ethaddr for intf in my_interfaces]
    # init the forwarding table
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

        # update the forwarding table upon receiving a packet (learn)
        table.update_in(eth.src, fromIface)

        if eth is None:
            log_info("Received a non-Ethernet packet?!")
            return
        if eth.dst in mymacs:
            log_info("Received a packet intended for me")
        
        else:
            # query the forwarding table for interface connected to dst
            dst_intf = table.get(eth.dst)
            if dst_intf != None and eth.dst != 'ff:ff:ff:ff:ff:ff':
                # update the forwarding table before sending a packet (update status)
                table.update_out(eth.dst, dst_intf)
                net.send_packet(dst_intf, packet)                
                log_info(f"Sending packet {packet} to {dst_intf}")
            else:
                for intf in my_interfaces:
                    if fromIface!= intf.name:
                        log_info(f"Flooding packet {packet} to {intf.name}")
                        net.send_packet(intf, packet)

    net.shutdown()