CC=clang
CFLAGS=-Wall -o3

all:
	cd raylib/src/; make PLATFORM=PLATFORM_DESKTOP
	$(CC) $(CFLAGS) gui.c main.c raylib/src/libraylib.a -o exe -lm -g -ggdb
clean:
	cd raylib/src/; make clean
	rm -rf exe
