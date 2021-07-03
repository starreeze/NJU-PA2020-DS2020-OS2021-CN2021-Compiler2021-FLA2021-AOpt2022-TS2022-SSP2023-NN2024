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
        #   blaster 
        #          \
        #           middlebox
        #          /
        #   blastee 
        #

        nodeconfig = {'cpu':-1}
        self.addHost('blaster', **nodeconfig)
        self.addHost('blastee', **nodeconfig)
        self.addHost('middlebox', **nodeconfig)

        linkconfig = {
            'bw': 10,
            'delay': '37.5ms', # Feel free to play with this value to see how it affects the communication
            'loss': 0.0
        }

        for node in ['blaster','blastee']:
            self.addLink(node, 'middlebox', **linkconfig)

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
    '''
    * reset_macs call sets the MAC address of the nodes in the network
    * blaster and blastee has a single port, hence the MAC address ends with :01
    * middlebox has two ports, MAC address ends with :01 and :02 respectively, that are connected to the blaster and blastee.
    '''
    reset_macs(net, 'blaster', '10:00:00:00:00:{:02x}')
    reset_macs(net, 'blastee', '20:00:00:00:00:{:02x}')
    reset_macs(net, 'middlebox', '40:00:00:00:00:{:02x}')
    '''
    * set_ip_pair call assigns IP addresses of the interfaces
    * convention is same as MAC address
    * middlebox has two IP addresses: 192.168.100.2 and 192.168.200.2 - connected to blaster and blastee respectively
    '''
    set_ip_pair(net, 'blaster','middlebox','192.168.100.1/30','192.168.100.2/30')
    set_ip_pair(net, 'blastee','middlebox','192.168.200.1/30','192.168.200.2/30')
    set_route(net, 'blaster', '192.168.200.0/24', '192.168.100.2')
    set_route(net, 'blastee', '192.168.100.0/24', '192.168.200.2')

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
