def __init__(self, interfaces: list) -> None:
    for intf in interfaces:
        self.data.append([ip_interface(str(intf.ipaddr) + '/' + str(intf.netmask)).network, IPv4Address('0.0.0.0'), intf.name])
    with open('forwarding_table.txt', 'r') as fp:
        lines = fp.readlines()
    # [network address, subnet address, next hop address, interface]
    for line in lines:
        strs = line.split(' ')
        if strs[3][-1] == '\n':
            strs[3] = strs[3][:-1]
        self.data.append([IPv4Network(strs[0] + '/' + strs[1]), IPv4Address(strs[2]), strs[3]])
    # sort by length: longest match
    self.data.sort(key=lambda entry: int(entry[0].netmask), reverse=True)