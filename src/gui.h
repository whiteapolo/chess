#ifndef GUI_H
#define GUI_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../raylib/src/raylib.h"
#include "config.h"
#include "macros.h"

typedef struct {
	char piece;
	bool highlight;
	u8 x;
	u8 y;
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
void GuiInitWindow(const char *fen);
bool GuiDrawWindow();
void GuiCloseWindow();

// return true if loading was successfull
// false otherwise
bool GuiLoadFen(const char *fen);

// return true if there was a move
bool GuiGetUserMove(char mv[4]);
void GuiMakeMove(char *mv);
char *GuiGetFen();

#endif
