#!/bin/sh

set -xe
gcc voronoi_ppm.c -o voronoi_ppm -Wall -Wextra
gcc nearestX_ppm.c -o nearestX_ppm -Wall -Wextra