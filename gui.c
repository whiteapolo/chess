#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "assets/font.h"
#include "config.h"
#include "raylib.h"
#include "types_and_macros.h"
#include "fen.h"

#define WHITE_PLR 1
#define BLACK_PLR 0

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

static Board b;
static Move move;

static Font font;
static Sound move_sound;
static Sound capture_sound;
static Texture2D pieces_textures[12];

#define PADX 			((GetScreenWidth() - (DIM * TILE_SIZE)) / 2)
#define PADY 			((GetScreenHeight() - (DIM * TILE_SIZE)) / 2)
#define IS_TILE_EMPTY(tile) 	(!isalpha((tile).piece))

#define DRAW_TILE(v,c) 		DrawRectangleV(v,(Vector2){TILE_SIZE, TILE_SIZE},c)
#define DRAW_PIECE(v,t) 	DrawTextureV(*textureByPiece(t),v,PIECES_TINT);
#define DRAW_LABEL(x,y,l,c) 	DrawTextEx(font,(char[]){l,'\0'},(Vector2){x,y},FONT_SIZE,0,c);
#define DRAW_INACTIVE_MASK() 	DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), INACTIVE_C)

static void loadAssets();
static void initBoard();
static void handleTouch();
static void drawBoard();
static Vector2 Vector2Sub(Vector2 v1, Vector2 v2);
static Vector2 tileToPos(Tile *tile);
static Tile *posToTile(Vector2 pos);
static char pieceToPlayer(char piece);
static Texture *textureByPiece(char piece);

void GuiInitWindow(const char *fen)
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);
	SetWindowMinSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
	InitAudioDevice();
	loadAssets();
	SetTargetFPS(FPS);
	initBoard();
	if (!GuiLoadFen(fen))
		exit(1);
}

void GuiDrawWindow()
{
	BeginDrawing();
	drawBoard();
	ClearBackground(BG_COLOR);
	handleTouch();
	EndDrawing();
}

void GuiCloseWindow()
{
	CloseWindow();
}

// LOCAL IMPLEMENTATION
//------------------------------------------------------------------
static void loadAssets()
{
	Image img;

	char pieces_path[] = PIECES_PATH;
	char *pieces_n = strchr(pieces_path, '$');

	// load pieces images
	FOR (i, sizeof(pieces)-1) {
		*pieces_n = pieces[i];
		img = LoadImage(pieces_path);
		ImageResize(&img, PIECE_SIZE, PIECE_SIZE);
		PRINT_VAR(pieces[i]);
		*textureByPiece(pieces[i]) = LoadTextureFromImage(img);
	}

	// load sound
	move_sound = LoadSound(SOUND_PATH "m.mp3");
	capture_sound = LoadSound(SOUND_PATH "c.mp3");

	// load font
	font = LoadCustomFont();
}

static void drawBoard()
{
	Vector2 tile_pos;
	Tile *curr_tile;

	// draw tiles
	FOR (i, DIM) {
		FOR (j, DIM) {
			curr_tile = &b.mat[i][j];
			Color tile_color = IS_EVEN(i+j) ? B_T_C : W_T_C;
			tile_pos.x = j * TILE_SIZE + PADX;
			tile_pos.y = i * TILE_SIZE + PADY;

			DRAW_TILE(tile_pos, tile_color);
			// Draw highlighted tiles
			if (curr_tile->highlight)
				DRAW_TILE(tile_pos, S_T_C);
		}
	}
	// Draw the pieces
	FOR (i, DIM) {
		FOR (j, DIM) {
			curr_tile = &b.mat[i][j];
			// if there is a piece here
			if (curr_tile->piece) {
				tile_pos.x = (j * TILE_SIZE + PADX) + ((TILE_SIZE - PIECE_SIZE) / 2);
				tile_pos.y = (i * TILE_SIZE + PADY) + ((TILE_SIZE - PIECE_SIZE) / 2);

				// calculate the animation offset
				if (curr_tile->offset.x)
					curr_tile->offset.x -= curr_tile->offset.x / 4;
				if (b.mat[i][j].offset.y)
					curr_tile->offset.y -= curr_tile->offset.y / 4;

				tile_pos = Vector2Sub(tile_pos, curr_tile->offset);

				DRAW_PIECE(tile_pos, curr_tile->piece);
			}
		}
		// draw the numbers on the edge of the board
		DRAW_LABEL(PADX+1, PADY+((DIM-i-1)*TILE_SIZE), '1'+i, IS_EVEN(i) ? B_T_C : W_T_C);
		DRAW_LABEL(PADX+((DIM-i)*TILE_SIZE)-10, PADY + DIM*TILE_SIZE - 17, 'h'-i, IS_EVEN(i) ? W_T_C : B_T_C);
	}
}

// return true if there was a click
bool getMouseClick(Vector2 *pos)
{
	*pos = GetMousePosition();
	if (IsMouseButtonPressed(0))
		return true;
	return false;
}

bool inBoard(Vector2 pos)
{
	return IN_BOUNDRY(pos.x, PADX, TILE_SIZE * DIM + PADX) &&
		IN_BOUNDRY(pos.y, PADY, TILE_SIZE * DIM + PADY);
}

static void handleTouch()
{
	Vector2 pos;

	if (!getMouseClick(&pos))
		return;

	// check if the click was on board
	if (!inBoard(pos))
		return;

	Tile *pressed_tile = posToTile(pos);
		
	// same tile as before was selected... 
	// deselecting it...
	if (pressed_tile == move.src) {
		move.src = NULL;
		pressed_tile->highlight = false;
	// if there a piece on tile and its the same color as the current player
	} else if (!IS_TILE_EMPTY(*pressed_tile) && pieceToPlayer(pressed_tile->piece) == b.player) {
		if (move.src != NULL)
			move.src->highlight = false;
		move.src = pressed_tile;
		move.src->highlight = true;
	// make the move.
	} else if (move.src != NULL) {
		move.dest = pressed_tile;
		// deheighlight the tiles
		move.src->highlight = false;
		move.dest->highlight = false;
	}
}

static void initBoard()
{
	b = (Board){0};
	move = (Move){0};
	FOR (i, DIM) {
		FOR (j, DIM) {
			b.mat[i][j].x = i;
			b.mat[i][j].y = j;
		}
	}
}

// check if a fen is legal
bool GuiLoadFen(const char *fen)
{
	char uncompressed_fen[100];
	char *cnt;
	char player;

	if (!FenIsValidFen(fen)) {
		printf("Chess: This Fen is not valid: \"%s\"\n", fen);
		return false;
	}

	FenUncompress(uncompressed_fen, fen, &player);
	cnt = uncompressed_fen;

	FOR(i, DIM) {
		FOR(j, DIM) {
			if (*cnt == EMPTY_SQUARE)
				b.mat[i][j].piece = 0;
			else
				b.mat[i][j].piece = *cnt;
			cnt++;
		}
	}

	b.player = (player == 'w' ? WHITE_PLR : BLACK_PLR);
	return true;
}

void GuiMakeMove(char *mv)
{
	// convert mv to a Move
	move.src = &b.mat[-(mv[0] - '8')][mv[1] - 'a'];
	move.dest = &b.mat[-(mv[2] - '8')][mv[3] - 'a'];

	// play sound
	move.dest->piece ? PlaySound(capture_sound) : PlaySound(move_sound);

	// make the actual move
	move.dest->piece = move.src->piece;
	move.src->piece = 0;

	// calculate the animation offset
	Vector2 destv = tileToPos(move.dest);
	Vector2 srcv = tileToPos(move.src);
	move.dest->offset = Vector2Sub(destv, srcv);

	// if the move was a promoting pawn
	// promote it...
	if (tolower(move.dest->piece) == 'p' && (move.dest->x == 0 || move.dest->x == DIM-1))
		move.dest->piece = b.player == WHITE_PLR ? 'Q' : 'q';

	move.dest = NULL;
	move.src = NULL;

	// switch players
	b.player = (b.player == WHITE_PLR ? BLACK_PLR : WHITE_PLR);
}

// return true if there was a move
bool GuiGetUserMove(char mv[4])
{
	if (move.dest == NULL)
		return false;

	mv[0] = '8' - move.src->x;
	mv[1] = 'a' + move.src->y;
	mv[2] = '8' - move.dest->x;
	mv[3] = 'a' + move.dest->y;
	move.src = NULL;
	move.dest = NULL;
	return true;
}

void GuiGetFen(char dest[100])
{
	FenBuilder fb;
	FenBuilderInit(&fb);

	FOR(i, DIM) {
		FOR(j, DIM) {
			FenBuilderAppendPiece(&fb, b.mat[i][j].piece);
		}
	}

	FenBuilderAppendPlayer(&fb, b.player);
	strcpy(dest, fb.data);
}

static Vector2 Vector2Sub(Vector2 v1, Vector2 v2)
{
	Vector2 v3;
	v3.x = v1.x - v2.x;
	v3.y = v1.y - v2.y;
	return v3;
}

static char pieceToPlayer(char piece)
{
	return ((bool)isupper(piece)) ? WHITE_PLR : BLACK_PLR;
}

static Vector2 tileToPos(Tile *tile)
{
	Vector2 v;
	v.x = tile->y * TILE_SIZE + PADX;
	v.y = tile->x * TILE_SIZE + PADY;
	return v;
}

static Tile *posToTile(Vector2 pos)
{
	u8 x = (pos.x - PADX) / TILE_SIZE;
	u8 y = (pos.y - PADY) / TILE_SIZE;
	return &b.mat[y][x];
}

static Texture *textureByPiece(char piece)
{
	switch (piece) {
		case 'P': return &pieces_textures[0];
		case 'N': return &pieces_textures[1];
		case 'B': return &pieces_textures[2];
		case 'R': return &pieces_textures[3];
		case 'Q': return &pieces_textures[4];
		case 'K': return &pieces_textures[5];
		case 'p': return &pieces_textures[6];
		case 'n': return &pieces_textures[7];
		case 'b': return &pieces_textures[8];
		case 'r': return &pieces_textures[9];
		case 'q': return &pieces_textures[10];
		case 'k': return &pieces_textures[11];
		default: return NULL;
	}
}
