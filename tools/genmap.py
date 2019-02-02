import sys
import math
import random

radius = 160
min_distance = 79
map_command_format = '    BUILDING_N(X, Y, Z) \\'
map_command_prefix = '#define MAP_GENERATED_MACRO \\\n'
floor_y = 2


class RInt:
    def __init__(self, rrange):
        self.random_int = int(math.floor(random.random()*rrange*rrange) % rrange)


def find_locations(rangeX, rangeY, rangeZ, max_proximity, add_location_function):
    R = (rangeX[1]-rangeX[0], rangeY[1]-rangeY[0], rangeZ[1]-rangeZ[0])

    if R[0] <= max_proximity or R[1] <= max_proximity or R[2] <= max_proximity:
        x = rangeX[0]+RInt(R[0]).random_int
        y = floor_y
        z = rangeZ[0]+RInt(R[2]).random_int
        add_location_function(x, y, z)
        return

    find_locations((rangeX[0], rangeX[1]-R[0]/2), rangeY, (rangeZ[0], rangeZ[1]-R[2]/2), max_proximity, add_location_function)
    find_locations((rangeX[0], rangeX[1]-R[0]/2), rangeY, (rangeZ[0]+R[2]/2, rangeZ[1]), max_proximity, add_location_function)
    find_locations((rangeX[0]+R[0]/2, rangeX[1]), rangeY, (rangeZ[0], rangeZ[1]-R[2]/2), max_proximity, add_location_function)
    find_locations((rangeX[0]+R[0]/2, rangeX[1]), rangeY, (rangeZ[0]+R[2]/2, rangeZ[1]), max_proximity, add_location_function)
    return


def add_building(x, y, z):
    N = 1+RInt(3).random_int
    h = float(random.random() * 314159) / 100000
    sys.stdout.write('BUILDING_'+str(N)+'('+str(x)+', '+str(y)+', '+str(z)+', '+str(h)+') \\\n')

sys.stdout.write(map_command_prefix)

find_locations((-radius, radius), (-radius, radius), (-radius, radius), min_distance, add_building)

sys.stdout.write('#endif\n')
sys.stdout.write('\n')
