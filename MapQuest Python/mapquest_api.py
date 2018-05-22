# Brian Huang 3458152
#
# ICS 32 Winter 2016

import json
import http.client
import urllib.request
import urllib.parse

MAPQUEST_API_KEY = 'efWP4CXhaPse1TXk4ApIfFjJjQARszUB'

BASE_MAPQUEST_URL = 'http://open.mapquestapi.com/'

'''
http://open.mapquestapi.com/directions/v2/route?key=YOUR_KEY_HERE
&ambiguities=ignore&from=Lancaster,PA&to=York,PA&callback=renderNarrative
'''

def build_route_url(locations: [str]) -> str:
    ''' Takes a list of locations to travel from and to, and buils and returns
        a URL that can be used to ask the Mapquest Data API for information on the route.
    '''

    route_parameters = [
        ('key', MAPQUEST_API_KEY), ('ambiguities', 'ignore')
    ]

    for i in range(0, len(locations)):
        if i == 0:
            route_parameters.append( ('from', locations[i]) )
        else:
            route_parameters.append( ('to', locations[i]) )

    return BASE_MAPQUEST_URL + 'directions/v2/route?' + urllib.parse.urlencode(route_parameters)

def build_elevation_url(latitude: float, longitude: float) -> str:
    ''' Takes a latitude and longitude, and buils and returns
        a URL that can be used to ask the Mapquest Data API for information on the elevation.
    '''
    elevation_parameters = [
        ('key', MAPQUEST_API_KEY), ('shapeFormat', 'raw'),
        ('unit', 'f'), ('latLngCollection', str(latitude) + ',' + str(longitude))
    ]

    return BASE_MAPQUEST_URL + 'elevation/v1/profile?' + urllib.parse.urlencode(elevation_parameters) 

def get_json_result(url: str) -> 'json':
    '''
    Takes a URL and returns a Python object representing the
    parsed JSON response.
    '''

    response = None

    try:
        response = urllib.request.urlopen(url)
        json_text = response.read().decode(encoding = 'utf-8')

        return json.loads(json_text)

    finally:
        if response != None:
            response.close()

class RouteErrorException(Exception):
    pass


def check_valid_response(reponse: 'json') -> None:
    ''' Checks to ensure there was no route error when sending locations to Mapquest;
        Raises a RouteError if there is one.
    '''
    if reponse['info']['statuscode'] == 400:
        raise RouteErrorException
    else:
        pass
        

    
