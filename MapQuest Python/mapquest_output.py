# Brian Huang 34586152
#
# ICS 32 Winter 2016

import mapquest_api

def build_output(output_type: str, json_response: 'json') -> 'output object':
    ''' Takes a string parameter for what output object to be created and a json
        object for data and creates an output object.
    '''
    if output_type == 'STEPS':
        return steps(json_response)
    elif output_type == 'TOTALDISTANCE':
        return totaldistance(json_response)
    elif output_type == 'TOTALTIME':
        return totaltime(json_response)
    elif output_type == 'LATLONG':
        return latlong(json_response)
    elif output_type == 'ELEVATION':
        return elevation(json_response)
    else:
        print('Invalid output type entered')
        return

class steps:
    def __init__(self, json_response: 'json'):
        '''Initializes the steps object to have the list of directions'''
        self._directions = []
        for leg in json_response['route']['legs']:
            for maneuver in leg['maneuvers']:
                self._directions.append(maneuver['narrative'])

    def output(self) -> str:
        output = 'DIRECTIONS \n'
        for i in range(0, len(self._directions)):
            if i == len(self._directions)-1:
                output += self._directions[i]
            else:
                output += self._directions[i] + '\n'
        return output

class totaldistance:
    def __init__(self, json_response: 'json'):
        '''Initializes the totaldistance object to have an integer value
           of the total distance of the route'''
        self._distance = 0
        
        self._distance += float((json_response['route']['distance']))

        self._distance = int(round(self._distance))

    def output(self) -> str:
        return 'TOTAL DISTANCE: ' + str(self._distance) + ' miles'
        
class totaltime:
    def __init__(self, json_response: 'json'):
        '''Initializes the totaltime object to have an integer value
           of the estimated total time of the route'''
        self._time = 0

        self._time += int(json_response['route']['time'])

        self._time = int(round(self._time/60))

    def output(self) -> str:
        return 'TOTAL TIME: ' + str(self._time) + ' minutes'

class latlong:
    def __init__(self, json_response: 'json'):
        '''Initializes the latlong object to have a list of latlong
           pairs of the locations in the route'''
        self._latlng_pairs = []
        
        for location in json_response['route']['locations']:
            self._latlng_pairs.append([
                    location['latLng']['lat'],
                    location['latLng']['lng']])
                    
                                        
    def _format_lat(lat: float) -> str:
        ''' Formats the latitude to two decimal places, and indicates whether it is N or S
        '''
        if lat > 0:
            return '{:.2f}N'.format(lat)
        elif lat < 0:
            return '{:.2f}S'.format(lat*(-1))
        else:
            return '{:.2f}'.format(lat)

    def _format_lng(lng: float) -> str:
        ''' Formats the longitude to two decimal places, and indicates whether it is W or E
        '''
        if lng > 0:
            return '{:.2f}E'.format(lng)
        elif lng < 0:
            return '{:.2f}W'.format(lng*(-1))
        else:
            return '{:.2f}'.format(lng)

    def get_latlng_pairs(self) -> [[float]]:
        ''' Returns the list of latitude and longitude pairs
        '''
        return self._latlng_pairs

    def output(self) -> str:
        output = 'LATLONGS\n'
        for i in range(0, len(self._latlng_pairs)):
            if i == len(self._latlng_pairs)-1:
                output += latlong._format_lat(self._latlng_pairs[i][0]) + ' ' + latlong._format_lng(self._latlng_pairs[i][1])
            else:          
                output += latlong._format_lat(self._latlng_pairs[i][0]) + ' ' + latlong._format_lng(self._latlng_pairs[i][1])
                output += '\n'
        return output

    
class elevation:
    def __init__(self, json_response: 'json'):
        '''Initializes the elevation object to have a list of
           of the elevations of the locations in the route'''
        self._elevations = []

        # Creates a latlong object that holds latlong pairs of all locations
        latlongs = latlong(json_response)

        # Creates a list of URLs to be used in the MapQuest elevation API
        elevation_urls = []
        for pair in latlongs.get_latlng_pairs():
            elevation_urls.append(mapquest_api.build_elevation_url(pair[0], pair[1]))

        # Using the URLs, creates json responses and records the height data for each location.
        for url in elevation_urls:
            j = mapquest_api.get_json_result(url)
            self._elevations.append(j['elevationProfile'][0]['height'])


    def output(self) -> str:
        output = 'ELEVATIONS\n'
        for i in range(0, len(self._elevations)):
            if i == len(self._elevations)-1:
                output += str(int(round(self._elevations[i])))
            else:
                output += str(int(round(self._elevations[i])))
                output += '\n'
        return output
