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
#include "move_set.h"
#include "raylib.h"
#include "types_and_macros.h"
#include "fen.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w"

char *fen = START_FEN;

// the only argument is the fen
void handleArguments(u32 argc, char **argv)
{
	if (argc > 2) {
		puts("Chess: Too many arguments");
		exit(1);
	}

	if (argc == 2) {
		fen = argv[1];
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
	bool mate = false;
	bool draw = false;

	handleArguments(argc, argv);
	GuiInitWindow(fen);
	BitboardInit(&bitboard, NULL);

	while (!WindowShouldClose() && !draw && !mate) {
		GuiDrawWindow();

		if (GuiGetUserMove(move)) {
			parseMove(move, &srck, &destk);
			if (BitboardIsValidMove(&bitboard, srck, destk, bitboard.current_player)) {
				BitboardMakeMove(&bitboard, srck, destk);
				GuiMakeMove(move);
				mate = BitboardIsMated(&bitboard, bitboard.current_player);
				draw = BitboardIsDraw(&bitboard);
			}
		}
	}

	GuiCloseWindow();
	return 0;
}

