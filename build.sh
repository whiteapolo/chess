#!/bin/sh
set -e

CC=gcc
CFLAGS="-Wall -O0 -g"
CFILES="fen.c gui.c main.c bitboard.c"
CLIBS="raylib/src/libraylib.a -lm"
EXENAME="exe"

compile()
{
	(cd raylib/src && make PLATFORM=PLATFORM_DESKTOP)
	$CC $CFLAGS generate_move_set.c -o generate_move_set
	./generate_move_set
	rm generate_move_set
	$CC $CFLAGS $1 $CFILES -o $EXENAME $CLIBS
}

profile()
{
	compile -pg
	./$EXENAME
	gprof ./$EXENAME gmon.out > $1
}

clean()
{
	(cd raylib/src && make clean)
	rm $EXENAME
}

if [ "$1" = "clean" ]; then
	clean
elif [ "$1" = "profile" ]; then
	if [ -n "$2" ]; then
		profile $2
		echo "profile: $2!"
	else
		echo "file name required!"
	fi
else
	compile
	echo "build succsesfuly!"
fi
