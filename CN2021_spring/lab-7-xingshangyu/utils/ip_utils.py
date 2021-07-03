import os
import pathlib
# import geoip2.webservice
# import geoip2.database

class IP_Utils:
    test_cases = {
        "127.0.0.1": (0,0),
        "10.0.0.1" : (50,0),
        "10.0.0.2" : (100,0),
        "10.0.0.3" : (200,0),
        "10.0.0.4" : (500,50),
        "10.0.0.5" : (-100,50),
    }

    @staticmethod
    def getIpLocation(ip_str):
        """ Read the latitude and Longitude of an ip address.
        ---------------------------------------------------------------
        Args:
            ip_str: ip address in string format.
        Returns:
            - (latitude, longitude) : location tuple
            - None : the ip address do not exist in the database.
        If the function returns None, You should randomly select an ip address.
        """
        if ip_str in IP_Utils.test_cases.keys():
            return IP_Utils.test_cases[ip_str]
        raise ValueError(f"IP address {ip_str} is not in location databse")
        # database = pathlib.Path(__file__).parent / 'data/GeoLite2-City.mmdb'
        # reader = geoip2.database.Reader(str(database))
        # try:
        #     response = reader.city(ip_str)
        #     latitude = response.location.latitude
        #     longitude = response.location.longitude
        # except geoip2.errors.AddressNotFoundError:
        #     return None
        # return (latitude, longitude)
