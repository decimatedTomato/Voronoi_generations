#!/bin/sh

set -xe
mkdir -p bin
# gcc voronoi_ppm.c -o bin/voronoi_ppm -Wall -Wextra
# ./bin/voronoi_ppm
# gcc nearestX_ppm.c -o bin/nearestX_ppm -Wall -Wextra
# ./bin/nearestX_ppm
# gcc distance_weighting_ppm.c -o bin/distance_weighting_ppm -Wall -Wextra
gcc distance_squared_weighting_ppm.c -o bin/distance_squared_weighting_ppm -Wall -Wextra
./bin/distance_squared_weighting_ppm