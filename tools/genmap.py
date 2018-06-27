import sys
import math
import random

radius = 100
gridsize = 64
grid = [ [0 for i in range(gridsize)] for i in range(gridsize) ]
map_command_format = '    BUILDING_N(X, Y, Z) \\'
map_command_prefix = '#define MAP_GENERATED_MACRO \\\n'
floor_y = 2.0

class RInt:
    def __init__(self, rrange):
        self.random_int = int(math.floor(random.random()*rrange*rrange) % rrange)

l = 0
while l < gridsize / 3:
    x = RInt(gridsize).random_int
    y = RInt(gridsize).random_int
    grid[x][y] = RInt(2).random_int+1
    l += 1


for y in range(gridsize):
    sys.stdout.write('//')
    for x in range(gridsize):
        sys.stdout.write(' ')
        sys.stdout.write(str(grid[x][y]))
    sys.stdout.write('\n')

sys.stdout.write(map_command_prefix)
for y in range(gridsize):
    for x in range(gridsize):
        if grid[x][y] != 0:
            U = radius*2 / gridsize
            map_cmd = map_command_format.replace('X', str(-radius + U * x)).replace('Y', str(floor_y)).replace('Z', str(-radius + U * y)).replace('_N', '_'+str(grid[x][y]))
            sys.stdout.write(map_cmd)
            sys.stdout.write('\n')
sys.stdout.write('\n')
