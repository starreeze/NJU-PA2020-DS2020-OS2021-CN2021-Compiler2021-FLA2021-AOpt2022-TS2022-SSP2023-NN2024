''' CacheTable used by CachingServer.

!NOTICE! You should NOT change this file!

CacheTable is a dict-like table storing <path, CacheItem>.

CacheTable is stored in memory, but may be implemented to use disk in the
future.
'''
import time
from collections import UserDict

from utils.tracer import trace


__all__ = ["HTTPCacheItem", "CacheTable"]


class HTTPCacheItem:
    ''' The value of the CacheTable '''
    def __init__(self, headers: list, body: bytearray):
        ''' Initiate an item that stores info of an HTTP response.
        headers: HTTP headers
        body: HTTP body
        timestamp: created time of this item. Used for expiration.
        '''
        self.headers = headers  # list of pairs
        self.body = body
        self.timestamp = time.time()  # to see if it's expired


class CacheTable(UserDict):
    ''' A dict-based cache table storing <path, CacheItem>.

    Example:
        >>> ct = CacheTable()
        >>> ct.setHeaders(path, headers)
        >>> ct.appendBody(path, body)
        >>> headers = ct.getHeaders(path)
        >>> body = ct.getBody(path)
    '''
    def __init__(self, timeout=-1):
        ''' Initiate a CacheTable.
        Params:
            timeout: seconds for a item to live. Negative for forever.
        '''
        self.timeout = timeout  # seconds. None for no timeout
        super().__init__()

    @trace
    def setHeaders(self, key: str, headers):
        ''' Set the headers of `key`. Create a item if `key` doesn't exist.
        Params:
            key: str, key to visit
            headers: List[Tuple[str, str]], headers to store
        '''
        if key in self.data:
            self.data[key].headers = headers
        else:
            self.data[key] = HTTPCacheItem(headers, bytearray())

    @trace
    def getHeaders(self, key: str):
        ''' Get headers of `key` item.
        Returns:
            List[Tuple[str, str]] headers.
        '''
        return self.data[key].headers

    def appendBody(self, key: str, body: bytearray):
        ''' Append the body to the CacheItem corresponding to key. 
        `key` should already in self.data, which means this should be called
        after calling self.setHeaders().
        '''
        self.data[key].body.extend(body)

    def getBody(self, key: str) -> bytearray:
        return self.data[key].body

    def expired(self, key):
        ''' Check if the item of `key` expired. Return True if expired. '''
        if self.timeout > 0:
            if time.time() - self.data[key].timestamp > self.timeout:
                del self.data[key]  # remove key
                return True
        return False
