all:
	gcc -Wall -o3 main.c -o exe raylib/libraylib-linux.a -lm
windows:
	gcc -Wall -o3 main.c -o exe raylib/libraylib-windows.a -lm
