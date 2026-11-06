CC=cc
CFLAGS=-Wall -O3 -g

all:
	cd raylib/src/; make PLATFORM=PLATFORM_DESKTOP
	$(CC) $(CFLAGS) src/gui.c src/main.c src/bitboard.c raylib/src/libraylib.a -o chess -lm

clean:
	cd raylib/src/; make clean
	rm -rf chess
