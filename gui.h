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

#define WHITE_PLR  		'w'
#define BLACK_PLR  		'b'

// misc
#define IN_BOUNDRY(p,p1,p2) 	(p.x >= p1.x && p.x <= p2.x && p.y >= p1.y && p.y <= p2.y)
#define HASH(c)			((c-1)%21)
#define PADX 			((GetScreenWidth() - (DIM * TILE_SIZE)) / 2)
#define PADY 			((GetScreenHeight() - (DIM * TILE_SIZE)) / 2)
#define IN_BOARD(pos) 		IN_BOUNDRY(pos,((Vector2){PADX,PADY}),((Vector2){PADX+TILE_SIZE*DIM, PADY+TILE_SIZE*DIM}))
#define GET_POS_FROM_TILE(tile, start) ((Vector2){(tile).y * TILE_SIZE + start.x, (tile).x * TILE_SIZE + start.y})
#define GET_TILE_FROM_POS(pos, start) ((Vector2){(pos.x - start.x)/TILE_SIZE, (pos.y - start.y)/TILE_SIZE})
#define GET_PLR(c) 		(isupper(c.piece) ? 'w' : 'b')
#define IS_TILE_EMPTY(tile) 	(!isalpha(tile.piece))
#ifndef ASSERT
	#define ASSERT(a, msg) if(a) {printf("ERROR: %s\n", msg); exit(1);}
#endif

// drawing
#define DRAW_TILE(v,c) 		DrawRectangleV(v,(Vector2){TILE_SIZE, TILE_SIZE},c)
#define DRAW_PIECE(v,t) 	if(HASH(t)) DrawTextureV(textures[HASH(t)],v,PIECES_TINT);
#define DRAW_LABEL(x,y,l,c) 	DrawTextEx(font,(char[]){l,'\0'},(Vector2){x,y},FONT_SIZE,0,c);
#define DRAW_INACTIVE_MASK() 	DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), INACTIVE_C)

// sound
#define MOVE_SOUND() 		PlaySound(move_sound)
#define CAPTURE_SOUND() 	PlaySound(capture_sound)

// Vector2
#define SUB_VECTOR2(v1, v2) ((Vector2){(v1).x - (v2).x, (v1).y - (v2).y})
#define ADD_VECTOR2(v1, v2) ((Vector2){(v1).x + (v2).x, (v1).y + (v2).y})

// DEBUGGING
#define PRINT_XY(x, y) 		printf("\tx = %d, y = %d\n",x,y)

Font font;
Sound move_sound;
Sound capture_sound;
Texture2D textures[19];
const char pieces[] = "prbnqkPRBNQK";
#define PIECES_COUNT  12
#define PAWN_PROMOTE_PIECES_WHITE "QRBN"
#define PAWN_PROMOTE_PIECES_BLACK "qrbn"

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
void init_window();
void draw_window(Board *b, Move *move);
void close_window();
void init_board(Board *b);
void load_fen_to_board(Board *b, const char *fen);

// LOCAL FUNCTIONS
//------------------------------------------------------------------
void draw_label(char c, ushort x, ushort y, Color color);
void draw_board(Board *b);
void load_assets();
void handle_touch(Board *b, Move *move);

// PUBLIC IMPLEMENTATION
//------------------------------------------------------------------

void init_window()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(SCREEN_WIDTH, SCREEN_WIDTH, WINDOW_NAME);
	InitAudioDevice();
	SetWindowMinSize(MIN_SCREEN_WIDTH, MIN_SCREEN_HEIGHT);
	load_assets();
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
void load_assets()
{
	Image img;

	char pieces_path[] = PIECES_PATH;
	char sound_path[] = SOUND_PATH;
	char *pieces_n = strchr(pieces_path, '$');
	char *sound_n = strchr(sound_path, '$');

	ASSERT(!pieces_n, "Please check your pieces path.");
	ASSERT(!sound_n, "Please check your sound path.");

	// load pieces images
	for (uchar i = 0; i < PIECES_COUNT; i++) {
		*pieces_n = pieces[i];
		img = LoadImage(pieces_path);
		ImageResize(&img, TILE_SIZE, TILE_SIZE);
		textures[HASH(pieces[i])] = LoadTextureFromImage(img);
	}

	// load sound
	*sound_n = 'm';
	move_sound = LoadSound(sound_path);
	*sound_n = 'c';
	capture_sound = LoadSound(sound_path);

	// load font
	font = LoadFont(FONT_PATH);
}

void draw_board(Board *b)
{
	short j, i;
	Vector2 pos;

	// draw tiles
	for (i = 0; i < DIM; i++) {
		for (j = 0; j < DIM; j++) {
			pos = (Vector2){j*TILE_SIZE+PADX, i*TILE_SIZE+PADY};
			DRAW_TILE(pos, (i+j)%2 ? B_T_C : W_T_C);
			// Draw highlighted tiles
			if (b->mat[i][j].highlight)
				DRAW_TILE(pos, S_T_C);
		}
	}
	// Draw the pieces
	for (i = 0; i < DIM; i++) {
		for (j = 0; j < DIM; j++) {
			pos = (Vector2){j*TILE_SIZE+PADX, i*TILE_SIZE+PADY};

			// draw the animation offset
			if (b->mat[i][j].offset.x)
				b->mat[i][j].offset.x -= b->mat[i][j].offset.x / 4;
			if (b->mat[i][j].offset.y)
				b->mat[i][j].offset.y -= b->mat[i][j].offset.y / 4;

			pos = SUB_VECTOR2(pos, b->mat[i][j].offset);

			DRAW_PIECE(pos, b->mat[i][j].piece);
		}
	}

	// draw labels
}

char open_promoting_menu(Board *b)
{
	char selected = 0;
	Vector2 pos;
	Vector2 startPos;
	Vector2 endPos;
	Vector2 offset = (Vector2){0,TILE_SIZE*2};

	char *pawn_promote_pieces = PAWN_PROMOTE_PIECES_WHITE;
	if (b->player == 'b')
		pawn_promote_pieces = PAWN_PROMOTE_PIECES_BLACK;

	while (!WindowShouldClose() && !selected) {
		BeginDrawing();
		startPos = (Vector2){PADX+2*TILE_SIZE, PADY+3.5*TILE_SIZE};
		endPos = (Vector2){PADX+6*TILE_SIZE, PADY+4.5*TILE_SIZE};
		if (offset.y)
			offset.y -= offset.y / 4;
		pos = ADD_VECTOR2(startPos, offset);
		draw_board(b);
		DRAW_INACTIVE_MASK();
		for (int i = 0; i < 4; i++) {
			DRAW_TILE(pos, (i%2) ? B_T_C : W_T_C);
			DRAW_PIECE(pos, pawn_promote_pieces[i]);
			pos.x += TILE_SIZE;
		}
		ClearBackground(BG_COLOR);
		if (IsMouseButtonPressed(0)) {
			pos = GetMousePosition();
			if (IN_BOUNDRY(pos, startPos, endPos))
				selected = pawn_promote_pieces[(int)GET_TILE_FROM_POS(pos, startPos).x];
		}
		EndDrawing();
	}
	return selected;
}

void handle_touch(Board *b, Move *move)
{
	// check if there was a click
	if (!IsMouseButtonPressed(0))
		return;

	Vector2 pos = GetMousePosition();

	// check if the click was on board
	if (!IN_BOARD(pos))
		return;

	// calculate the tile that was clicked
	pos = GET_TILE_FROM_POS(pos, ((Vector2){PADX,PADY}));
	int x = pos.x;
	int y = pos.y;

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

void init_board(Board *b)
{
	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			b->mat[i][j].x = i;
			b->mat[i][j].y = j;
			/* b->mat[i][j].offset = (Vector2){0}; */
			/* b->mat[i][j].highlight = false; */
		}
	}
}

// asuming the fen is legal
void load_fen_to_board(Board *b, const char *fen)
{
	for (int i = 0; *fen != ' '; fen++) {
		if (isdigit(*fen)) {
			i += *fen - '0';
		}
		else if (isalpha(*fen)) {
			b->mat[i/8][i%8].piece = *fen;
			i++;
		}
	}
	b->player = *(++fen);
}

#endif
