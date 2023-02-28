#!/usr/bin/env python3

import re
import time
import socket
import functools
import urllib.request
import urllib.error
from urllib.parse import urlparse, urlunparse
from utils.dns_utils import DNS_Request, DNS_Response


def timer(func):
    '''Decorator: a timer '''
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        tick = time.time()
        ret = func(*args, **kwargs)
        toc = time.time()
        print(f"\n[Request time] {(toc - tick) * 1000:.2f} ms")
        return ret
    return wrapper


def isIpv4(text):
    if not re.match(r"\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}", text):
        return False
    splitted = text.split(".")
    for num in splitted:
        if int(num) < 0 or int(num) > 255:
            return False
    return True


def resolve_domain_name(url, dnsIP, dnsPort):
    # SOCK_DGRAM is the socket type to use for UDP sockets
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(10)
        # Send a request to dns server.
        dns_request = DNS_Request.construct_dns_request(url)
        if dns_request is None:
            raise ValueError(f"cannot create DNS request from URL '{url}'")
        sock.sendto(dns_request.to_bytes(), (dnsIP, dnsPort))
        received = sock.recv(1024)
        dns_response = DNS_Response(received)
        if dns_response.response_type is not None:
            return dns_response
    return None


@timer
def _request(req):
    try:
        with urllib.request.urlopen(req) as f:
            return f.status, f.read()
    except urllib.error.HTTPError as e:
        return e.getcode(), None


def curl(url, dnsIP=None, dnsPort=None, method="GET"):
    o = urlparse(url)
    if not isIpv4(o.hostname):
        # url has a domain name
        resp = resolve_domain_name(o.hostname, dnsIP, dnsPort)
        if resp is None:
            # cannot resolve domain name
            raise ValueError("could not resolve domain name")
        while resp.response_type == 5:
            nd = resp.response_val
            resp = resolve_domain_name(nd, dnsIP, dnsPort)
        if resp.response_type != 1:
            # cannot resolve domain name
            raise ValueError("could not resolve domain name")
        url = url.replace(o.hostname, str(resp.response_val))

    # now url is an IP address
    req = urllib.request.Request(url)
    req.get_method = lambda: method
    return _request(req)


def createUrl(scheme="http", netloc="127.0.0.1", port=None,
              path="", params="", query="", fragment=""):
    addr = f"{netloc}:{port}" if port else netloc
    urlParts = (scheme, addr, path, params, query, fragment)
    return urlunparse(urlParts)
