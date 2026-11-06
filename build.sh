#!/bin/sh
set -xe
cd raylib/src; make PLATFORM=PLATFORM_DESKTOP
cd ../..
gcc -Wall -o3 gui.c main.c bitboard.c raylib/src/libraylib.a -o exe -lm -g
