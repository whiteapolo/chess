#ifndef GUI_H
#define GUI_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "raylib.h"

#define WHITE_PLR  		1
#define BLACK_PLR  		0

#define DIM1 			8
#define DIM2 			8
#define T_W 	   		100
#define T_H 	   		T_W

#define SCREEN_W 		T_W*DIM1
#define SCREEN_H 		T_W*DIM2
#define MIN_SCREEN_W 		T_W*DIM1
#define MIN_SCREEN_H 		T_H*DIM2
#define WINDOW_NAME 		"chess"
#define FPS 	   		60
#define FONT_PATH  		"assets/JetBrainsMono-Bold.ttf"
#define PIECES_PATH 		"assets/$.png"
#define FONT_SIZE  		24

#define BG_COLOR     		CLITERAL(Color){0x42, 0x54, 0x5f}
#define W_T_C        		CLITERAL(Color){0xc7, 0xc5, 0xa6, 0xff}
#define B_T_C        		CLITERAL(Color){0x6d, 0x7b, 0x7b, 0xff}
#define S_T_C        		CLITERAL(Color){0xdb, 0xb5, 0x5a, 0x88}
#define PIECES_TINT  		CLITERAL(Color){0xeb, 0xe5, 0xcf, 0xff}

#define ASSERT(a, msg) 		if(a) {printf("ERROR: %s\n", msg); exit(1);}
#define HASH(c)			((c-1)%21)
#define PADX 			((GetScreenWidth() - (DIM1 * T_W)) / 2)
#define PADY 			((GetScreenHeight() - (DIM2 * T_H)) / 2)
#define ON_BOARD(x,y) 		((unsigned)x-PADX < T_W*DIM1 && (unsigned)y-PADY < T_H*DIM2)
#define DRAW_TILE(x,y,c) 	DrawRectangle(x,y,T_W,T_H,c)
#define DRAW_PIECE(x, y, t) 	if(HASH(t)) DrawTexture(textures[HASH(t)],x,y,PIECES_TINT);
#define DRAW_LABEL(x,y,l,c) 	DrawTextEx(font,(char[]){l,'\0'},(Vector2){x,y},FONT_SIZE,0,c);
#define GET_PLR(c) 		((bool)isupper(c.piece))
#define IS_TILE_EMPTY(a) 	(!isalpha(a.piece))

// require board to be a power of 2
//#define ON_BOARD(x,y) 		(((unsigned)((x-PADX) |  (y-PADY))) < ((T_W*DIM1) | (T_H*DIM2)))

// DEBUGGING
#define PRINT_XY(x, y) 		printf("\tx = %d, y = %d\n",x,y)

Font font;
Texture2D textures[19];
const char pieces[] = "prbnqkPRBNQK";
#define PIECES_COUNT  12

typedef struct {
	char piece;
	bool highlight;
	unsigned char x;
	unsigned char y;
} Tile;

typedef struct {
	Tile **mat;
	bool player;
} Board;

typedef struct {
	Tile *src;
	Tile *dest;
} Move;


// PUBLIC FUNCTIONS
//------------------------------------------------------------------
void init_window();
void draw_window(Board *b, Move *move);
void close_window();

// LOCAL FUNCTIONS
//------------------------------------------------------------------
void draw_label(char c, int x, int y, Color color);
void draw_board(Board *b);
void load_textures();
void handle_touch(Board *b, Move *move);
bool in_board(int x, int y);

// PUBLIC IMPLEMENTATION
//------------------------------------------------------------------

void init_window()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(SCREEN_W, SCREEN_W, WINDOW_NAME);
	SetWindowMinSize(MIN_SCREEN_W, MIN_SCREEN_H);
	load_textures();
	font = LoadFont(FONT_PATH);
	SetTargetFPS(FPS);
}

// print the board and puts the player move
// inside the var move
// if no move were made than move will 
// not be changed
void draw_window(Board *b, Move *move)
{
	BeginDrawing();
	draw_board(b);
	ClearBackground(BG_COLOR);
	handle_touch(b, move);
	EndDrawing();
}

void close_window()
{
	CloseWindow();
}

// LOCAL IMPLEMENTATION
//------------------------------------------------------------------
void load_textures()
{
	// Load the pieces pictures
	Image img;
	char file_path[] = PIECES_PATH;
	// find where to change the file name
	char *x = strchr(file_path, '$');
	ASSERT(!x, "Please check your pieces img path.");
	for (int i = 0; i < PIECES_COUNT; i++) {
		*x = pieces[i];
		img = LoadImage(file_path);
		ImageResize(&img, T_W, T_H);
		textures[HASH(pieces[i])] = LoadTextureFromImage(img);
	}
}

void draw_board(Board *b)
{
	int j, i;

	for (i = DIM1-1; i >= 0; i--) {
		for (j = DIM2-1; j >= 0; j--) {
			// Draw the tiles
			DRAW_TILE(j*T_W+PADX, i*T_H+PADY, (i+j)%2 ? B_T_C : W_T_C);
			// Draw highlighted tiles
			if (b->mat[i][j].highlight)
				DRAW_TILE(j*T_W+PADX, i*T_H+PADY, S_T_C);
			// Draw the pieces
			DRAW_PIECE(j*T_W+PADX, i*T_H+PADY, b->mat[i][j].piece);
		}
		// draw the labels
		DRAW_LABEL(PADX+3, i*T_H+PADY+3, '8'-i, (i%2) ? W_T_C : B_T_C);
		DRAW_LABEL((i+1)*T_W+PADX-14, DIM2*T_H+PADY-20, 'a'+i, (i%2) ? B_T_C : W_T_C);
	}
}

void handle_touch(Board *b, Move *move)
{
	// check if there was a click
	if (!IsMouseButtonPressed(0))
		return;

	int x = GetMouseX();
	int y = GetMouseY();

	// check if the click was on board
	if (!ON_BOARD(x, y))
		return;

	// calculate the tile that was clicked
	x = (x - PADX) / T_W;
	y = (y - PADY) / T_H;

	// same tile as before was selected... 
	// deselecting it...
	if (&b->mat[y][x] == move->src) {
		move->src = NULL;
		b->mat[y][x].highlight = false;
	// if there a piece on tile and its the same color as the current player
	} else if (!IS_TILE_EMPTY(b->mat[y][x]) && GET_PLR(b->mat[y][x]) == b->player) {
		if (move->src != NULL)
			move->src->highlight = false;
		move->src = &b->mat[y][x];
		move->src->highlight = true;
	// make the move.
	} else if (move->src != NULL) {
		move->dest = &b->mat[y][x];
		// deheighlight the tiles
		move->src->highlight = false;
		move->dest->highlight = false;
	}
}

#endif
