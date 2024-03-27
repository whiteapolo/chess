#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "engine.h"
#include "gui.h"
#include "raylib.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define SWITCH_PLAYERS(a) (a = (a == 'w' ? 'b' : 'w'))

#ifndef ASSERT
	#define ASSERT(a, msg) if(a) {printf("ERROR: %s\n", msg); exit(1);}
#endif

char fen[256] = START_FEN;

void mk_move(Board *board, Move *move)
{
	// play move sound or capture sound
	// depends if there is already a piece in dest
	move->dest->piece ? CAPTURE_SOUND() : MOVE_SOUND();
	move->dest->piece = move->src->piece;
	move->src->piece = 0;
	// calculate the animation offset
	move->dest->offset = SUB_VECTOR2(GET_POS_FROM_TILE(*(move->dest), ((Vector2){PADX, PADY})),
					GET_POS_FROM_TILE(*(move->src), ((Vector2){PADX, PADY})));

	// if the move was a promoting pawn
	// promote it...
	if (tolower(move->dest->piece) == 'p' && (move->dest->x == 0 || move->dest->x == DIM-1))
		move->dest->piece = open_promoting_menu(board);
}

// the only argument is the fen
void handle_arguments(int argc, char **argv)
{
	int i = 0;
	while (++i < argc) {
		strcpy(fen, argv[i]);
	}
}

int main(int argc, char **argv)
{
	handle_arguments(argc, argv);
	Board b = (Board){0};
	Move move = (Move){0};
	init_board(&b);
	ASSERT(!is_legal_fen(fen, DIM), "fen isn't valid");
	load_fen_to_board(&b, fen);
	init_window();
	while (!WindowShouldClose()) {
		draw_window(&b, &move);
		// check if a move was made
		if (move.dest != NULL) {
			mk_move(&b, &move);
			SWITCH_PLAYERS(b.player);
			// reset the move
			move = (Move){0};
		}
		sleep(GetFrameTime());
	}
	close_window();
	return 0;
}
