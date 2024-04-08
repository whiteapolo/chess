#include "gui.h"
#include "assets/font.h"
#include "config.h"
#include "raylib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define WHITE_PLR  		'w'
#define BLACK_PLR  		'b'
#define SWITCH_PLAYERS() 	(b.player = (b.player == 'w' ? 'b' : 'w'))

#define IN_BOUNDRY(p,p1,p2) 	(p.x >= p1.x && p.x <= p2.x && p.y >= p1.y && p.y <= p2.y)
#define HASH(c)			((c-1)%21)
#define PADX 			((GetScreenWidth() - (DIM * TILE_SIZE)) / 2)
#define PADY 			((GetScreenHeight() - (DIM * TILE_SIZE)) / 2)
#define IN_BOARD(pos) 		IN_BOUNDRY(pos,((Vector2){PADX,PADY}),((Vector2){PADX+TILE_SIZE*DIM,PADY+TILE_SIZE*DIM}))
#define GET_POS_FROM_TILE(tile,start) ((Vector2){(tile).y*TILE_SIZE+start.x,(tile).x*TILE_SIZE+start.y})
#define GET_TILE_FROM_POS(pos,start) ((Vector2){(pos.x-start.x)/TILE_SIZE,(pos.y-start.y)/TILE_SIZE})
#define GET_PLR(c) 		(isupper(c.piece) ? 'w' : 'b')
#define IS_TILE_EMPTY(tile) 	(!isalpha(tile.piece))

#define SUB_VECTOR2(v1, v2) 	((Vector2){(v1).x - (v2).x, (v1).y - (v2).y})
#define ADD_VECTOR2(v1, v2) 	((Vector2){(v1).x + (v2).x, (v1).y + (v2).y})

// drawing
#define DRAW_TILE(v,c) 		DrawRectangleV(v,(Vector2){TILE_SIZE, TILE_SIZE},c)
#define DRAW_PIECE(v,t) 	DrawTextureV(textures[HASH(t)],v,PIECES_TINT);
#define DRAW_LABEL(x,y,l,c) 	DrawTextEx(font,(char[]){l,'\0'},(Vector2){x,y},FONT_SIZE,0,c);
#define DRAW_INACTIVE_MASK() 	DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), INACTIVE_C)

#define FOR_RANGE(var, from, to, jumps) for (int var = from; var < to; var+=jumps)
#define MALLOC(type, n) malloc(sizeof(type) * n)
#define IS_EVEN(num) ((num)&1)


#define ASSERT(a, msg) if(a) {printf("ERROR: %s\n", msg); exit(1);}

Board b = (Board){0};
Move move = (Move){0};

Font font;
Sound move_sound;
Sound capture_sound;
Texture2D textures[19];
const char pieces[] = "prbnqkPRBNQK";
#define PIECES_COUNT  12
#define PAWN_PROMOTE_PIECES_WHITE "QRBN"
#define PAWN_PROMOTE_PIECES_BLACK "qrbn"

// DEBUGGING
#define PRINT_XY(x, y) 		printf("\tx = %d, y = %d\n",x,y)

void load_assets();
bool load_fen(const char *fen);
bool is_legal_fen(const char *fen);
void init_board();
bool handle_touch();
char open_promoting_menu();
void draw_board();
void mk_move();

void init_window(const char *fen)
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME);
	SetWindowMinSize(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
	InitAudioDevice();
	load_assets();
	SetTargetFPS(FPS);
	init_board();
	if (!load_fen(fen))
		exit(1);
}

bool draw_window()
{
	BeginDrawing();
	draw_board();
	ClearBackground(BG_COLOR);
	/* bool move_made = handle_touch(); */
	EndDrawing();
	return handle_touch();
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
	char *pieces_n = strchr(pieces_path, '$');

	// load pieces images
	FOR_RANGE(i, 0, PIECES_COUNT, 1) {
		*pieces_n = pieces[i];
		img = LoadImage(pieces_path);
		ImageResize(&img, PIECE_SIZE, PIECE_SIZE);
		textures[HASH(pieces[i])] = LoadTextureFromImage(img);
	}

	// load sound
	move_sound = LoadSound(SOUND_PATH "m.mp3");
	capture_sound = LoadSound(SOUND_PATH "c.mp3");

	// load font
	font = LoadCustomFont();
}

void draw_board()
{
	Vector2 pos;

	// draw tiles
	FOR_RANGE(i, 0, DIM, 1) {
		FOR_RANGE(j, 0, DIM, 1) {
			pos = (Vector2){j*TILE_SIZE+PADX, i*TILE_SIZE+PADY};
			DRAW_TILE(pos, IS_EVEN(i+j) ? B_T_C : W_T_C);
			// Draw highlighted tiles
			if (b.mat[i][j].highlight)
				DRAW_TILE(pos, S_T_C);
		}
	}
	// Draw the pieces
	FOR_RANGE(i, 0, DIM, 1) {
		FOR_RANGE(j, 0, DIM, 1) {
			// if there is a piece here
			if (b.mat[i][j].piece) {
				pos = (Vector2){j*TILE_SIZE+PADX, i*TILE_SIZE+PADY};
				// center the piece in the tile
				pos.x += (TILE_SIZE-PIECE_SIZE)/2;
				pos.y += (TILE_SIZE-PIECE_SIZE)/2;

				// calculate the animation offset
				if (b.mat[i][j].offset.x)
					b.mat[i][j].offset.x -= b.mat[i][j].offset.x / 4;
				if (b.mat[i][j].offset.y)
					b.mat[i][j].offset.y -= b.mat[i][j].offset.y / 4;
				pos = SUB_VECTOR2(pos, b.mat[i][j].offset);

				DRAW_PIECE(pos, b.mat[i][j].piece);
			}
		}
		// draw the numbers on the edge of the board
		DRAW_LABEL(PADX+1, PADY+((DIM-i-1)*TILE_SIZE), '1'+i, IS_EVEN(i) ? B_T_C : W_T_C);
		DRAW_LABEL(PADX+((DIM-i)*TILE_SIZE)-10, PADY + DIM*TILE_SIZE - 17, 'h'-i, IS_EVEN(i) ? W_T_C : B_T_C);
	}
}

char open_promoting_menu()
{
	char selected = 0;
	Vector2 pos, startPos, endPos;
	Vector2 offset = (Vector2){0,TILE_SIZE*2};

	char *pawn_promote_pieces = (b.player == 'b') ?
		PAWN_PROMOTE_PIECES_BLACK : PAWN_PROMOTE_PIECES_WHITE;

	while (!WindowShouldClose() && !selected) {
		BeginDrawing();
		startPos = (Vector2){PADX+2*TILE_SIZE, PADY+3.5*TILE_SIZE};
		endPos = (Vector2){PADX+6*TILE_SIZE, PADY+4.5*TILE_SIZE};
		if (offset.y)
			offset.y -= offset.y / 4;
		pos = ADD_VECTOR2(startPos, offset);
		draw_board();
		DRAW_INACTIVE_MASK();
		FOR_RANGE(i, 0, 4, 1) {
			DRAW_TILE(pos, IS_EVEN(i) ? B_T_C : W_T_C);
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

bool handle_touch()
{
	// check if there was a click
	if (!IsMouseButtonPressed(0))
		return false;

	Vector2 pos = GetMousePosition();

	// check if the click was on board
	if (!IN_BOARD(pos))
		return false;

	// calculate the tile that was clicked
	pos = GET_TILE_FROM_POS(pos, ((Vector2){PADX,PADY}));
	uchar x = pos.x;
	uchar y = pos.y;

	// same tile as before was selected... 
	// deselecting it...
	if (&b.mat[y][x] == move.src) {
		move.src = NULL;
		b.mat[y][x].highlight = false;
	// if there a piece on tile and its the same color as the current player
	} else if (!IS_TILE_EMPTY(b.mat[y][x]) && GET_PLR(b.mat[y][x]) == b.player) {
		if (move.src != NULL)
			move.src->highlight = false;
		move.src = &b.mat[y][x];
		move.src->highlight = true;
	// make the move.
	} else if (move.src != NULL) {
		move.dest = &b.mat[y][x];
		// deheighlight the tiles
		move.src->highlight = false;
		move.dest->highlight = false;
		return true;
	}
	return false;
}

void init_board()
{
	FOR_RANGE(i, 0, DIM, 1) {
		FOR_RANGE(j, 0, DIM, 1) {
			b.mat[i][j].x = i;
			b.mat[i][j].y = j;
		}
	}
}

// check if a fen is legal
bool is_legal_fen(const char *fen)
{
	int i = 0;
	int slash = 0;

	while (*fen && *fen != ' ') {
		if (isdigit(*fen))
			i += *fen - '0';
		else if (*fen == '/')
			slash++;
		else if (strchr("rnbqkp", tolower(*fen)))
			i++;
		else 
			return false;
		fen++;
	}
	if (!(*(fen++)) || i != 64 || slash != 8 - 1)
		return false;

	if (!strchr("wb", *(fen++)))
		return false;

	if (!(*(fen++)) && !(*fen + 1))
		return false;

	FOR_RANGE(j, 0, 4, 1)
		if (!strchr("-KQkq", *(fen++)))
			return false;

	if (*fen != '\0' && *fen != ' ')
		return false;

	return true;
}

bool load_fen(const char *fen)
{
	if (!is_legal_fen(fen)) {
		printf("This Fen is not valid! %s", fen);
		return false;
	}
	for (int i = 0; *fen != ' '; fen++) {
		if (isdigit(*fen)) {
			i += *fen - '0';
		}
		else if (isalpha(*fen)) {
			b.mat[i/8][i%8].piece = *fen;
			i++;
		}
	}
	b.player = *(++fen);
	return true;
}

void mk_move(char *mv)
{
	/* move.src->x = mv[0] - 8 - '0'; */
	/* move.src->y = mv[1] - 8 - 'a'; */
	/* move.dest->x = mv[2] - 8 - '0'; */
	/* move.dest->y = mv[3] - 8 - 'a'; */

	move.dest->piece ? PlaySound(capture_sound) : PlaySound(move_sound);
	move.dest->piece = move.src->piece;
	move.src->piece = 0;
	// calculate the animation offset
	move.dest->offset = SUB_VECTOR2(GET_POS_FROM_TILE(*(move.dest), ((Vector2){PADX, PADY})),
		GET_POS_FROM_TILE(*(move.src), ((Vector2){PADX, PADY})));
	// if the move was a promoting pawn
	// promote it...
	if (tolower(move.dest->piece) == 'p' && (move.dest->x == 0 || move.dest->x == DIM-1))
		move.dest->piece = open_promoting_menu();
	move = (Move){0};
	SWITCH_PLAYERS();
}

char *get_usr_move()
{
	if (move.dest != NULL) {
		char *mv = MALLOC(char, 5);
		mv[0] = '0' + 8 - move.src->x;
		mv[1] = 'a' + 8 - move.src->y;
		mv[2] = '0' + 8 - move.dest->x;
		mv[3] = 'a' + 8 - move.dest->y;
		mv[4] = '\0';
		return mv;
	}
	return NULL;
}

char *get_fen()
{
	char *fen = MALLOC(char, 100);
	char *start = fen;

	FOR_RANGE(i, 0, DIM, 1) {
		FOR_RANGE(j, 0, DIM, 1) {
			if (b.mat[i][j].piece == 0) {
				if (isdigit(*(fen-1))) {
					(*(fen-1))++;
					fen--;
				} else {
					*fen = '1';
				}
			} else {
				*fen = b.mat[i][j].piece;
			}
			fen++;
		}
		*(fen++) = '/';
	}
	*(fen-1) = ' ';
	*fen = b.player;
	*(fen+1) = '\0';
	return start;
}
