#ifndef GUI_H
#define GUI_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "raylib.h"
#include "config.h"

#define GAME_LOOP while (!WindowShouldClose())

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

typedef struct {
	char piece;
	bool highlight;
	uchar x;
	uchar y;
	Vector2 offset;
} Tile;

typedef struct {
	Tile mat[DIM][DIM];
	char player;
} Board;

typedef struct {
	Tile *src;
	Tile *dest;
} Move;


// PUBLIC FUNCTIONS
//------------------------------------------------------------------
void init_window(const char *fen);
bool draw_window();
void close_window();

// return true if loading was successfull
// false otherwise
bool load_fen(const char *fen);
char *get_usr_move();
void mk_move(char *mv);
char *get_fen();

#endif
