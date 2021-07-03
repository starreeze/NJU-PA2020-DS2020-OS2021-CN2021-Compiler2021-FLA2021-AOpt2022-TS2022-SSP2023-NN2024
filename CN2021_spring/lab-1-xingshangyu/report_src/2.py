while True:
    try:
        _, fromIface, packet = net.recv_packet()
    except NoPackets:
        continue
    except Shutdown:
        break
    in_count += 1
    log_debug (f"In {net.name} received packet {packet} on {fromIface}")
    eth = packet.get_header(Ethernet)
    if eth is None:
        log_info("Received a non-Ethernet packet?!")
        return
    if eth.dst in mymacs:
        log_info("Received a packet intended for me")
    else:
        for intf in my_interfaces:
            if fromIface != intf.name:
                log_info (f"Flooding packet {packet} to {intf.name}")
                net.send_packet(intf, packet)
                out_count += 1
    log_info("in:%d out:%d" % (in_count, out_count))