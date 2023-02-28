#!/usr/bin/env python

from utils.network import resolve_domain_name

dnsIP, dnsPort = "localhost", 9999

def resolveDomain(domain):
    resp = resolve_domain_name(domain, dnsIP, dnsPort)
    if resp:
        return resp.response_val
    return None

result = resolveDomain("home.nasa.org")
print(result)

result = resolveDomain("test.nasa.org")
print(result)
