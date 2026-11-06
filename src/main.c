#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "bitboard.h"
#include "gui.h"
#include "macros.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

char fen[256] = START_FEN;

// the only argument is the fen
void handleArguments(u32 argc, char **argv)
{
	if (argc > 2) {
		puts("Chess: Too many arguments\n");
		exit(1);
	}

	if (argc == 2) {
		strcpy(fen, argv[1]);
	}
}

void buildMove(char move[4], u8 src, u8 dest)
{
	move[0]	= '8' - (src / 8);
	move[1]	= 'a' + (src % 8);
	move[2]	= '8' - (dest / 8);
	move[3]	= 'a' + (dest % 8);
}

void parseMove(const char *move, u8 *srck, u8 *destk)
{
	*srck    = ('8' - move[0]) * 8;
	*srck   +=  move[1] - 'a';
	*destk   = ('8' - move[2]) * 8;
	*destk  +=  move[3] - 'a';
}

int main(int argc, char **argv)
{
	Bitboard bitboard;
	u8 srck, destk;
	char move[4];

	handleArguments(argc, argv);
	GuiInitWindow(fen);
	BitboardInit(&bitboard, NULL);

	while (!WindowShouldClose() && !BitboardIsMated(&bitboard, bitboard.current_player)) {

		GuiDrawWindow();

		if (GuiGetUserMove(move)) {
			parseMove(move, &srck, &destk);
			if (BitboardIsValidMove(&bitboard, srck, destk)) {
				BitboardMakeMove(&bitboard, srck, destk);
				GuiMakeMove(move);
			}
		}
	}

	GuiCloseWindow();
	return 0;
}
