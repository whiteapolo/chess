all:
	gcc -Wall -o3 main.c -o exe -lraylib
linux:
	gcc -Wall -o3 main.c -o exe -Wl,-rpath,raylib-bin/ -Lraylib-bin/ -lraylib
windows:
	gcc -Wall -o3 main.c -o exe -Wl,-rpath,raylib-bin/ -Lraylib-bin/ -lraylib
