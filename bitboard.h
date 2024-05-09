#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdbool.h>
#include "types_and_macros.h"

typedef enum Player {
	BLACK,
	WHITE,
} Player;

typedef struct {
	u64 b[15];
	Player current_player;
} Bitboard;

enum BoardType {
	WHITE_PAWNS, WHITE_KNIGHTS, WHITE_BISHOPS, 
	WHITE_ROOKS, WHITE_QUEENS, WHITE_KINGS,
	BLACK_PAWNS, BLACK_KNIGHTS, BLACK_BISHOPS,
	BLACK_ROOKS, BLACK_QUEENS, BLACK_KINGS,
	WHITE_ALL, BLACK_ALL, ALL,
};

void BitboardInit(Bitboard *b, const char *fen);
void BitboardMakeMove(Bitboard *b, u64 srck, u64 destk);
bool BitboardIsChecked(Bitboard *b, Player player);
bool BitboardIsMated(Bitboard *b, Player player);
bool BitboardIsDraw(Bitboard *b);
bool BitboardIsValidMove(Bitboard *b, u8 srck, u8 destk, Player player);

void getAllValidMoves(Bitboard *b, u8 moves[][2], u32 *len, u32 n, Player player);

void BitboardPrint(u64 b);
void BitboardPrintAll(Bitboard *b);

#endif
