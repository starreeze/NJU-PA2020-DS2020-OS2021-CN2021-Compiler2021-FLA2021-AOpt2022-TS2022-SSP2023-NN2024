from dnslib import *
from enum import Enum
import struct

class DNS_Rcode(Enum):
    NoError = 0       # No Error
    FormErr = 1       # Format Error
    ServFail = 2      # Server Failure 
    NXDomain = 3      # Non-Existent Domain
    # ... 
    # ignore other coeds.


class DNS_Request:
    """
    A simple dns toolkit wraped from dnslib.
    ------------------------------------------------------------------------
    NOTE: This module is only a partial of DNS protocol. As is mentioned, 
    it's just a wrapper from dnslib. The purpose of this module is to make 
    it easier for you to finish the lab, without caring many details on the
    protocol itself. We also do not consider about the performance issue.
    """
    def __init__(self, raw_data):
        self._raw_data = raw_data
        # Only support one question now.
        self._domain_name = DNSRecord.parse(raw_data).questions[0].get_qname()

    @property
    def domain_name(self):
        return self._domain_name

    @property
    def raw_data(self):
        return self._raw_data
    
    def to_bytes(self):
        return self._raw_data

    def generate_response(self, response):
        """
        This function generates a dns response for the current request.
        return: DNS_Response object
        qr : Query
        aa : Authoritative Answer
        ra : Recursion Available
        rd : Recursion Desired
        """
        if response[0] == "CNAME":
            return DNS_Response(DNSRecord(header=DNSHeader(qr=1,aa=1,ra=1, rcode=DNS_Rcode.NoError.value),
                                          q=DNSQuestion(self._domain_name),
                                          a=RR(self._domain_name, rtype=QTYPE.CNAME, rdata=CNAME(response[1]))).pack())
        elif response[0] == "A":
            return DNS_Response(DNSRecord(header=DNSHeader(qr=1,aa=1,ra=1, rcode=DNS_Rcode.NoError.value),
                                          q=DNSQuestion(self._domain_name),
                                          a=RR(self._domain_name, rdata=A(response[1]))).pack())

    @classmethod
    def generate_error_response(cls, error_code):
        """
        This function generates a dns response with specific error code.
        return: DNS_Response object
        usage: req_obj.generate_error_response(DNS_Rcode.FormErr)
        """
        return DNS_Response(DNSRecord(header=DNSHeader(qr=1,aa=1,ra=1, rcode=error_code.value)).pack())

    @classmethod
    def construct_dns_request(cls, domain_name):
        """
        This function constructs a dns request.
        return: DNS_Request object
        usage: DNS_Request.construct_dns_request("example.com")
        """
        raw_data = DNSRecord.question(domain_name).pack()
        return cls(raw_data)

    @staticmethod
    def check_valid_format(raw_data):
        try:
            d= DNSRecord.parse(raw_data)
            if len(d.questions) < 1:
                return False
        except DNSError as e:
            return False
        return True
    
class DNS_Response:
    def __init__(self, raw_data):
        self._raw_data = raw_data
        d = DNSRecord.parse(raw_data)
        self._rcode = d.header.get_rcode()
        self._response_type = None
        self._response_val = None
        if self._rcode == DNS_Rcode.NoError.value:
            self._response_type = d.a.rtype
            self._response_val = d.a.rdata    
            self._domain_name = d.questions[0].get_qname()

    @property
    def domain_name(self):
        return self._domain_name

    @property
    def response_type(self):
        return self._response_type

    @property
    def response_val(self):
        return self._response_val

    @property
    def raw_data(self):
        return self._raw_data