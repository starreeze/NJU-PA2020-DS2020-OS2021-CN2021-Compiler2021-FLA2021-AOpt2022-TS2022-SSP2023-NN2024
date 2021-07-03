def create_icmp(type, code, packet, srcip):
    # create ip
    hip = IPv4()
    hip.src = srcip
    hip.dst = packet.get_header(IPv4).src
    hip.ttl = 100
    hip.protocol = IPProtocol.ICMP
    # create icmp
    hicmp = ICMP()
    hicmp.icmptype = type
    hicmp.icmpcode = code
    return Ethernet() + hip + hicmp