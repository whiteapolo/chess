#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "bitboard.h"
#include "data_structers/graph.h"
#include "gui.h"
#include "move_set.h"
#include "raylib.h"
#include "types_and_macros.h"
#include "fen.h"
#include "engine.h"

#define USER_PLAY true
#define COMPUTER_PLAY false

// the only argument is the fen
void handleArguments(u32 argc, char **argv, char **fen)
{
	if (argc > 2) {
		puts("Chess: Too many arguments");
		exit(1);
	}

	if (argc == 2) {
		*fen = argv[1];
	}
}

void updateBoardStats(Bitboard *b, bool *mate, bool *draw)
{
	*mate = BitboardIsMated(b, b->current_player);
	*draw = BitboardIsDraw(b);
}

int main(int argc, char **argv)
{
	Graph g;
	Bitboard bitboard;
	char move[5];
	bool mate = false;
	bool draw = false;
	char *fen = START_FEN;

	handleArguments(argc, argv, &fen);
	GuiInitWindow(fen);
	BitboardInit(&bitboard, fen);
	initStateGraph(&g);

	mate = BitboardIsMated(&bitboard, bitboard.current_player);
	draw = BitboardIsDraw(&bitboard);

	while (!WindowShouldClose() && !draw && !mate) {
		GuiDrawWindow();

		if (bitboard.current_player == COMPUTER_PLAY) {
			playBestMove(&g, &bitboard, bitboard.current_player);
			updateBoardStats(&bitboard, &mate, &draw);
		} else {
			if (GuiGetUserMove(move)) {
				if (BitboardIsValidMoveAlgebraicNotation(&bitboard, move, bitboard.current_player)) {
					BitboardMakeMoveAlgebraicNotation(&bitboard, move);
					GuiMakeMove(move);
					updateBoardStats(&bitboard, &mate, &draw);
				}
			}
		}
	}

	while (!WindowShouldClose())
		GuiDrawWindow();

	GuiCloseWindow();
	return 0;
}
