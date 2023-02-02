#!/bin/sh

set -xe
# gcc voronoi_ppm.c -o voronoi_ppm -Wall -Wextra
# gcc nearestX_ppm.c -o nearestX_ppm -Wall -Wextra
gcc distance_weighting_ppm.c -o distance_weighting_ppm -Wall -Wextra
# ./distance_weighting_ppm