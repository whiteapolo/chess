#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "engine.h"
#include "gui.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define ASSERT(a, msg) if(a) {printf("ERROR: %s\n", msg); exit(1);}

char fen[256] = START_FEN;

// the only argument is the fen
void handle_arguments(int argc, char **argv)
{
	int i = 0;
	while (++i < argc)
		strcpy(fen, argv[i]);
}

int main(int argc, char **argv)
{
	char *mv;
	char *current_fen = START_FEN;
	handle_arguments(argc, argv);
	init_window(fen);
	current_fen = get_fen();
	printf("\nFEN: %s\n", current_fen);
	free(current_fen);

	GAME_LOOP {
		draw_window();
		mv = get_usr_move();
		if (mv) {
			// need if a move is legal
			printf("%s\n", mv);
			mk_move(mv);
			free(mv);

			current_fen = get_fen();
			printf("FEN: %s\n", current_fen);
			free(current_fen);
		}
	}
	close_window();
	return 0;
}
