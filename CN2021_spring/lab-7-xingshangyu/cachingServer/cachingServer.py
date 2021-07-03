''' Caching Server for Content Delivery Network (CDN)

CachingServer is a subclass of TCPServer that runs a server.
CachingServerHttpHandler is a subclass of BaseHTTPRequestHandler that handles
HTTP reqeust.

There is a CacheTable in CachingServer to store CDN caches.
CachingServer is also responsible for fetching content from remote main server.

If the target content does not exist in the server cache, the server should
fetch it from remote main server and store it in local cache for future usage.
Else the server shall just response with the cache content.

For optional task 2, we need to consider large content delivery. When we fetch
a large content from remote, storing it locally before replying back to client
is not acceptable, since the client will wait for a long time. So the server
shall store and response to client simultaneously.

'''

import io
import sys
from datetime import datetime
from typing import Type, Optional, Tuple, List
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler
from http.client import HTTPConnection, HTTPResponse
from socketserver import TCPServer

from .cacheTable import CacheTable
from utils.tracer import trace


__all__ = ["CachingServer", "CachingServerHttpHandler"]

__version__ = "0.1"

CACHE_TIMEOUT = 10

BUFFER_SIZE = 64 * 1024  # bytes. 64 KB


class CachingServer(TCPServer):
    ''' The caching server for CDN '''
    def __init__(self,
                 serverAddress:        Tuple[str, str],
                 serverRequestHandler: Type[BaseHTTPRequestHandler],
                 mainServerAddress:    str,
                 ):
        ''' Construct a server.
        Params:
            serverAddresss: the server's address
            serverRequestHandler: handler for http request. Subclass of
                BaseHTTPRequestHandler.
            mainServerAddress: the address(include port) to main server,
                e.g. 172.0.10.1:8080
        '''
        self.mainServerAddress = mainServerAddress
        self.cacheTable = CacheTable(timeout=CACHE_TIMEOUT)
        self.allow_reuse_address = True
        self.response = None
        self.buffer = bytearray(BUFFER_SIZE)
        super().__init__(serverAddress, serverRequestHandler, True)

    def _filterHeaders(self, headers: List[Tuple[str, str]]):
        ''' discard some headers and return the left '''
        discardHeaders = {"server", "date", "connection"}
        return [header for header in headers
                if header[0].lower() not in discardHeaders]

    @trace
    def requestMainServer(self, path: str) -> Optional[HTTPResponse]:
        ''' GET `path` from main server.
        Called by self.touchItem().
        Params:
            path: path of target
        Return:
            HTTPResponse if successfully requested.
            None if failed (server is down or file not found).
        '''
        conn = HTTPConnection(self.mainServerAddress)
        try:
            conn.request("GET", path)
        except ConnectionRefusedError:
            self.log_error(f"Cannot connect to main server '{self.mainServerAddress}'")
            return None
        response: HTTPResponse = conn.getresponse()
        if response.status == HTTPStatus.OK:
            self.log_info(f"Fetched '{path}' from main server "
                          f"'{self.mainServerAddress}'")
            return response

        # else: status isn't ok
        self.log_error(f"File not found on main server '{self.mainServerAddress}'")
        return None

    def touchItem(self, path: str):
        ''' Touch the item of path.
        This method, called by HttpHandler, serves as a bridge of server and
        handler.
        If the target doesn't exsit or expires, fetch from main server.
        Write the headers to local cache and return the body.
        '''
        assert(self.response is None)
        if path in self.cacheTable and not self.cacheTable.expired(path):
            return self.cacheTable.getHeaders(path), self.cacheTable.getBody(path)
        response = self.requestMainServer(path)
        if response is None:
            return None
        headers = response.getheaders()
        self.cacheTable.setHeaders(path, headers)
        self.response = response
        self.path = path
        return headers, None # to be continue in get_body

    def get_body(self):
        if self.response is None:
            raise StopIteration()
        length = self.response.readinto(self.buffer)
        if length < BUFFER_SIZE:
            self.response = None
        self.cacheTable.appendBody(self.path, self.buffer[:length])
        return self.buffer[:length]

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


class CachingServerHttpHandler(BaseHTTPRequestHandler):
    ''' A caching server for CDN network.
    An HTTP request or response should have a head and an optional body.
    
    The request head will be parsed automatically in BaseHTTPRequestHandler.
    The path in URL will be stored in self.path. It will call self.do_GET() or
    self.do_HEAD() according to the request's method. You can simply consider
    one of them the entry of the handler.
    
    The response head is consist of status, version and multiple headers. At
    least it should have headers "Content-Type" and "Content-Length". The
    former is the type of the content to send and the latter is how many bytes
    the content has. Also the BaseHTTPRequestHandler provides some useful
    methods to create the headers.

    There are two io.BufferedIOBase readable and writable objects, self.rfile
    and self.wfile. self.rfile is used to read bytes from the client and
    self.wfile is used to write bytes to the client.
    '''

    server_version = "CachingServerHTTP/" + __version__

    @trace
    def sendHeaders(self, headers):
        ''' Send HTTP headers to client'''
        self.send_response(HTTPStatus.OK)
        for header in headers:
            self.send_header(header[0], header[1])
        self.end_headers()

    def sendBody(self, body):
        ''' Send HTTP body to client.
        Should be called after calling self.sendHeaders(). Else you may get
        broken pipe error.
        '''
        self.wfile.write(body)

    @trace
    def do_GET(self):
        ''' Logic when receive a HTTP GET.
        Notice that the URL is automatically parsed and the path is stored in
        self.path. 
        '''
        res = self.server.touchItem(self.path)
        if res is None:
            self.send_error(HTTPStatus.NOT_FOUND, "'File not found'")
            return
        headers, body = res
        self.sendHeaders(headers)
        if body is None:
            try:
                while True:
                    self.sendBody(self.server.get_body())
            except StopIteration:
                pass
        else:
            self.sendBody(body)

    @trace
    def do_HEAD(self):
        ''' Logic when receive a HTTP HEAD.
        The difference from self.do_GET() is that do_HEAD() only send HTTP
        headers.
        '''
        res = self.server.touchItem(self.path)
        if res is None:
            self.send_error(HTTPStatus.NOT_FOUND, "'File not found'")
            return
        headers, _ = res
        self.sendHeaders(headers)

    def version_string(self):
        ''' Return the server software version string. '''
        return self.server_version

    def log_message(self, fmt, *args):
        ''' Override the method of base class '''
        info = f"[From {self.client_address[0]}:{self.client_address[1]}]"
        now = datetime.now().strftime("%Y/%m/%d-%H:%M:%S")
        sys.stdout.write(f"{now}| {info} {fmt % args}\n")
