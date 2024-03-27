all:
	cd raylib/src/; make PLATFORM=PLATFORM_DESKTOP
	gcc -Wall -o3 main.c raylib/src/libraylib.a -o exe -lm -g
clean:
	cd raylib/src/; make clean
	rm -rf exe
