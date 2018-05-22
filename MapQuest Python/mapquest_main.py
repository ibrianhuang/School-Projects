# Brian Huang 34586152
#
# ICS 32 Winter 2016

import mapquest_api
import mapquest_output

_SHOW_DEBUG = False

def run() -> None:
    ''' Runs the program.
    '''
    try:
        # Specifies how many locations the trip will consist of
        num_trips = int(input())
        if _SHOW_DEBUG:
            print(num_trips)


        # With n locations, the next n lines of input will describe one location
        locations = ask_inputs(num_trips)
        if _SHOW_DEBUG:
            print(locations)

        # Specifies how many outputs will need to be generated
        num_outputs = int(input())
        if _SHOW_DEBUG:
            print(num_outputs)

        # With m outputs, the next m lines of input will each describe one output
        outputs = ask_inputs(num_outputs)
        if _SHOW_DEBUG:
            print(outputs)

        # Creates a json response using the list of locations
        json_response = create_json_object(locations)

        # Checks that the json responses yielded valid locations/routes
        mapquest_api.check_valid_response(json_response)

        # Using the list of json responses, creates output objects
        output_objects = build_output_list(outputs, json_response)

        # Uses the common .output() function on all the output objects to print output
        for item in output_objects:
            print()
            print(item.output())
        
    except mapquest_api.RouteErrorException:
        print()
        print('NO ROUTE FOUND')
        
    except:
        print()
        print('MAPQUEST ERROR')
        
    else:
        # Prints the Mapquest copyright banner if the outputs went through properly.
        print_copyright_banner()
        

def ask_inputs(n: int) -> ['inputs']:
    ''' Takes n number of inputs and returns a list of the inputs '''
    inputs = []
    for i in range(0, n):
        inputs.append(input())

    return inputs

def create_json_object(locations: [str]) -> 'json':
    ''' Using a list of location strings, creates a Mapquest json resposne
        to be used for output data.
    '''
    url = mapquest_api.build_route_url(locations)
    return mapquest_api.get_json_result(url)

def build_output_list(outputs: [str], json_response: 'json') -> ['output objects']:
    ''' Using a list of strings describing output types and a json responses,
        creates a list of the output objects.
    '''
    output_objects = []
    for o in outputs:
        output_objects.append(mapquest_output.build_output(o, json_response))

    return output_objects

def print_copyright_banner() -> None:
    ''' Prints the closing copyright banner '''
    print()
    print('Directions Courtesy of MapQuest; Map Data Copyright OpenStreetMap Contributors')

if __name__ == '__main__':
    run()
