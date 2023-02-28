#!/usr/bin/python

import sys

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.log import lg
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import irange, custom, quietRun, dumpNetConnections
from mininet.cli import CLI

from time import sleep, time
from subprocess import Popen, PIPE
import subprocess
import argparse
import os

parser = argparse.ArgumentParser(description="Mininet portion of pyrouter")
# no arguments needed as yet :-)
args = parser.parse_args()
lg.setLogLevel('info')

class PyRouterTopo(Topo):

    def __init__(self, args):
        # Add default members to class.
        super(PyRouterTopo, self).__init__()

        # Host and link configuration
        #
        #
        #   server1 
        #          \
        #           router----client
        #          /
        #   server2 
        #

        nodeconfig = {'cpu':-1}
        self.addHost('server1', **nodeconfig)
        self.addHost('server2', **nodeconfig)
        self.addHost('router', **nodeconfig)
        self.addHost('client', **nodeconfig)

        linkconfig = {
            'bw': 10,
            'delay': 0.010,
            'loss': 0.0
        }

        for node in ['server1','server2','client']:
            self.addLink(node, 'router', **linkconfig)

def set_ip_pair(net, node1, node2, ip1, ip2):
    node1 = net.get(node1)
    ilist = node1.connectionsTo(net.get(node2)) # returns list of tuples
    intf = ilist[0]
    intf[0].setIP(ip1)
    intf[1].setIP(ip2)

def reset_macs(net, node, macbase):
    ifnum = 1
    node_object = net.get(node)
    for intf in node_object.intfList():
        node_object.setMAC(macbase.format(ifnum), intf)
        ifnum += 1

    for intf in node_object.intfList():
        print(node,intf,node_object.MAC(intf))

def set_route(net, fromnode, prefix, gw):
    node_object = net.get(fromnode)
    node_object.cmdPrint("route add -net {} gw {}".format(prefix, gw))

def setup_addressing(net):
    reset_macs(net, 'server1', '10:00:00:00:00:{:02x}')
    reset_macs(net, 'server2', '20:00:00:00:00:{:02x}')
    reset_macs(net, 'client', '30:00:00:00:00:{:02x}')
    reset_macs(net, 'router', '40:00:00:00:00:{:02x}')
    set_ip_pair(net, 'server1', 'router', '192.168.100.1/30', '192.168.100.2/30')
    set_ip_pair(net, 'server2', 'router', '192.168.200.1/30', '192.168.200.2/30')
    set_ip_pair(net, 'client', 'router', '10.1.1.1/30', '10.1.1.2/30')
    set_route(net, 'server1', '10.1.0.0/16', '192.168.100.2')
    set_route(net, 'server1', '192.168.200.0/24', '192.168.100.2')
    set_route(net, 'server2', '10.1.0.0/16', '192.168.200.2')
    set_route(net, 'server2', '192.168.100.0/24', '192.168.200.2')
    set_route(net, 'client', '192.168.100.0/24', '10.1.1.2')
    set_route(net, 'client', '192.168.200.0/24', '10.1.1.2')
    set_route(net, 'client', '172.16.0.0/16', '10.1.1.2')

    table = (
        "192.168.100.0 255.255.255.0 192.168.100.1 router-eth0\n"
        "192.168.200.0 255.255.255.0 192.168.200.1 router-eth1\n"
        "10.1.0.0 255.255.0.0 10.1.1.1 router-eth2"
    )
    with open('forwarding_table.txt', 'w') as fp:
        fp.write(table)

def disable_ipv6(net):
    for v in net.values():
        v.cmdPrint('sysctl -w net.ipv6.conf.all.disable_ipv6=1')
        v.cmdPrint('sysctl -w net.ipv6.conf.default.disable_ipv6=1')

def main():
    topo = PyRouterTopo(args)
    net = Mininet(topo=topo, link=TCLink, cleanup=True, controller=None)
    setup_addressing(net)
    disable_ipv6(net)
    net.interact()

if __name__ == '__main__':
    main()
