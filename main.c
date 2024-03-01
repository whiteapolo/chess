#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "gui.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
#define SWITCH_PLAYERS(a) (a = !a)

void free_board(Board *b);

Board *fen_to_board(char *fen)
{
	Board *b = malloc(sizeof(Board));
	b->player = WHITE_PLR;
	b->mat = malloc(sizeof(Tile*) * DIM1);
	int i, j;
	// alocate the columns of the board
	// and sets the x and y positions
	for (i = 0; i < DIM1; i++) {
		b->mat[i] = calloc(sizeof(Tile), DIM2);
		for (j = 0; j < DIM2; j++) {
			b->mat[i][j].x = i;
			b->mat[i][j].y = j;
		}
	}

	// convert the fen to the board
	i = 0;
	while (*fen) {
		if (isdigit(*fen))
			i += *fen - '0';
		else if (isalpha(*fen)) {
			b->mat[i/8][i%8].piece = *fen;
			i++;
		}
		fen++;
	}
	return b;
}

// tmp function
void make_move(Move *move)
{
	move->dest->piece = move->src->piece;
	move->src->piece = 0;
}

int main()
{
	Board *b = fen_to_board(START_FEN);
	Move move = (Move){0};
	init_window();
	while (!WindowShouldClose()) {
		draw_window(b, &move);
		if (move.dest != NULL) {
			make_move(&move);
			printf("%c to x = %d, y = %d\n", move.dest->piece, move.dest->x, move.dest->y);
			SWITCH_PLAYERS(b->player);
			// reset the move
			move = (Move){0};
		}
	}
	free_board(b);
	close_window();
	return 0;
}


void free_board(Board *b)
{
	for (int i = 0; i < DIM1; i++)
		free(b->mat[i]);
	free(b->mat);
	free(b);
}

