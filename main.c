#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "gui.h"
#include "raylib.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
#define SWITCH_PLAYERS(a) (a = !a)

void init_board(Board *b)
{
	b->player = WHITE_PLR;
	int i, j;
	// sets the x and y positions
	for (i = 0; i < DIM; i++) {
		for (j = 0; j < DIM; j++) {
			b->mat[i][j].x = i;
			b->mat[i][j].y = j;
			b->mat[i][j].offset = (Vector2){0};
			b->mat[i][j].highlight = false;
		}
	}
}

bool is_legal_fen(const char *fen)
{
	int tmp = 0;

	while (*fen) {
		while (*fen != '/' && *fen) {
			if (!strchr("rnbqkp", tolower(*fen)) || !isdigit(*fen)) {
				return false;
			}
			if (strchr("rnbqkp", tolower(*fen)))
				tmp++;
			if (isdigit(*fen))
				tmp += *fen - '0';
			fen++;
		}
		if (tmp != DIM)
			return false;
		tmp = 0;
		fen++;
	}
	return true;
}

void load_fen_to_board(Board *b, const char *fen)
{
	// convert the fen to the board
	int i = 0;
	while (*fen) {
		if (isdigit(*fen))
			for (int j = 0; j < *fen - '0'; j++, i++)
				b->mat[i/8][i%8].piece = 0;
		else if (isalpha(*fen)) {
			b->mat[i/8][i%8].piece = *fen;
			i++;
		}
		fen++;
	}
}

void mk_move(Board *board, Move *move)
{
	// play move sound or capture sound
	// depends if there is already a piece in dest
	move->dest->piece ? CAPTURE_SOUND() : MOVE_SOUND();
	move->dest->piece = move->src->piece;
	move->src->piece = 0;
	move->dest->offset = SUB_VECTOR2(GET_POS_FROM_TILE(*(move->dest), ((Vector2){PADX, PADY})),
					GET_POS_FROM_TILE(*(move->src), ((Vector2){PADX, PADY})));

	// if the move was a promoting pawn
	// promote it...
	if (tolower(move->dest->piece) == 'p' && (move->dest->x == 0 || move->dest->x == DIM-1))
		move->dest->piece = open_promoting_menu(board);
}

int main()
{


	/* printf("%d\n", is_legal_fen(START_FEN)); */
	/* exit(1); */


	Board b;
	Move move = (Move){0};
	init_board(&b);
	load_fen_to_board(&b, START_FEN);
	init_window();
	/* SetClipboardText(""); */
	while (!WindowShouldClose()) {
		draw_window(&b, &move);
		// check if a move was made
		if (move.dest != NULL) {
			mk_move(&b, &move);
			SWITCH_PLAYERS(b.player);
			// reset the move
			move = (Move){0};
		}
		/* if (strcmp(GetClipboardText(), "")) { */
		/* 	load_fen_to_board(&b, GetClipboardText()); */
		/* 	SetClipboardText(""); */
		/* } */
		sleep(GetFrameTime());
	}
	close_window();
	return 0;
}
