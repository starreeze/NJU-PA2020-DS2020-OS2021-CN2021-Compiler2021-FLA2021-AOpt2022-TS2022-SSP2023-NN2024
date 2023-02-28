'''DNS Server for Content Delivery Network (CDN)
'''

import sys
from socketserver import UDPServer, BaseRequestHandler
from utils.dns_utils import DNS_Request, DNS_Rcode
from utils.ip_utils import IP_Utils
from datetime import datetime
import math
import random


__all__ = ["DNSServer", "DNSHandler"]


class DNSServer(UDPServer):
    def __init__(self, server_address, dns_file, RequestHandlerClass, bind_and_activate=True):
        super().__init__(server_address, RequestHandlerClass, bind_and_activate=True)
        self.dns_table = []
        self.parse_dns_file(dns_file)
        
    def parse_dns_file(self, dns_file):
        # ---------------------------------------------------
        # TODO: your codes here. Parse the dns_table.txt file
        # and load the data into self._dns_table.
        # --------------------------------------------------
        with open(dns_file, mode='r') as file:
            content = file.read()
            if(content[-1] == '\n'):
                content = content[:-1]
            for line in content.split('\n'):
                items = line.split(' ')
                for i in range(len(items)):
                    if items[i][-1] == '.':
                        items[i] = items[i][:-1]
                self.dns_table.append(items)

    @property
    def table(self):
        return self.dns_table


class DNSHandler(BaseRequestHandler):
    """
    This class receives clients' udp packet with socket handler and request data. 
    ----------------------------------------------------------------------------
    There are several objects you need to mention:
    - udp_data : the payload of udp protocol.
    - socket: connection handler to send or receive message with the client.
    - client_ip: the client's ip (ip source address).
    - client_port: the client's udp port (udp source port).
    - DNS_Request: a dns protocl tool class.
    We have written the skeleton of the dns server, all you need to do is to select
    the best response ip based on user's infomation (i.e., location).

    NOTE: This module is a very simple version of dns server, called global load ba-
          lance dns server. We suppose that this server knows all the ip addresses of 
          cache servers for any given domain_name (or cname).
    """
    
    def __init__(self, request, client_address, server):
        self.table = server.table
        super().__init__(request, client_address, server)

    @staticmethod
    def calc_distance(lng1, lat1, lng2, lat2):
        ''' TODO: calculate distance between two points '''
        # lng1, lat1, lng2, lat2 = map(math.radians, [float(lng1), float(lat1), float(lng2), float(lat2)])
        # dlon = lng2 - lng1
        # dlat = lat2 - lat1
        # a = math.sin(dlat/2)**2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon/2)**2
        # distance = 2 * math.asin(math.sqrt(a)) * 6371 * 1000
        # distance = round(distance / 1000, 3)
        # return distance
        return (lng1 - lng2) ** 2 + (lat1 - lat2) ** 2

    @staticmethod
    def match(dst, src) -> bool:
        i, j = len(dst) - 1, len(src) - 1
        while i >= 0 and j >= 0:
            if dst[i] == '*':
                return True
            if dst[i] != src[j]:
                return False
            i -= 1
            j -= 1
        return i == j

    def select_ip(self, lst, clientip):
        pc = IP_Utils.getIpLocation(str(clientip))
        if pc is None:
            return random.choice(lst)
        dist = math.inf
        for sip in lst:
            ps = IP_Utils.getIpLocation(sip)
            if ps is None:
                return random.choice(lst)
            tmp_dist = self.calc_distance(pc[1], pc[0], ps[1], ps[0])
            if tmp_dist < dist:
                res = sip
                dist = tmp_dist
        return res

    def get_response(self, request_domain_name):
        response_type, response_val = (None, None)
        # ------------------------------------------------
        # TODO: your codes here.
        # Determine an IP to response according to the client's IP address.
        #       set "response_ip" to "the best IP address".
        client_ip, _ = self.client_address
        if request_domain_name[-1] == '.':
            request_domain_name = request_domain_name[:-1]
        for entry in self.table:
            if self.match(entry[0], request_domain_name):
                response_type = entry[1]
                if len(entry) == 3:
                    response_val = entry[2]
                else:
                    response_val = self.select_ip(entry[2:], client_ip)
                break
        # -------------------------------------------------
        return (response_type, response_val)

    def handle(self):
        """
        This function is called once there is a dns request.
        """
        ## init udp data and socket.
        udp_data, socket = self.request

        ## read client-side ip address and udp port.
        client_ip, client_port = self.client_address

        ## check dns format.
        valid = DNS_Request.check_valid_format(udp_data)
        if valid:
            ## decode request into dns object and read domain_name property.
            dns_request = DNS_Request(udp_data)
            request_domain_name = str(dns_request.domain_name)
            self.log_info(f"Receving DNS request from '{client_ip}' asking for "
                          f"'{request_domain_name}'")

            # get caching server address
            response = self.get_response(request_domain_name)

            # response to client with response_ip
            if None not in response:
                dns_response = dns_request.generate_response(response)
            else:
                dns_response = DNS_Request.generate_error_response(
                                             error_code=DNS_Rcode.NXDomain)
        else:
            self.log_error(f"Receiving invalid dns request from "
                           f"'{client_ip}:{client_port}'")
            dns_response = DNS_Request.generate_error_response(
                                         error_code=DNS_Rcode.FormErr)

        socket.sendto(dns_response.raw_data, self.client_address)

    def log_info(self, msg):
        self._logMsg("Info", msg)

    def log_error(self, msg):
        self._logMsg("Error", msg)

    def log_warning(self, msg):
        self._logMsg("Warning", msg)

    def _logMsg(self, info, msg):
        ''' Log an arbitrary message.
        Used by log_info, log_warning, log_error.
        '''
        info = f"[{info}]"
        now = datetime.now().strftime("%Y/%m/%d-%H:%M:%S")
        sys.stdout.write(f"{now}| {info} {msg}\n")
