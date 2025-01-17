#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdbool.h>
#include "types_and_macros.h"

typedef bool Player;
#define WHITE_PLR 1
#define BLACK_PLR 0

#define MAX_MOVES 218
#define NULL_PIECE 64


enum BoardType {
	WHITE_PAWNS, WHITE_KNIGHTS, WHITE_BISHOPS, 
	WHITE_ROOKS, WHITE_QUEENS, WHITE_KINGS,
	BLACK_PAWNS, BLACK_KNIGHTS, BLACK_BISHOPS,
	BLACK_ROOKS, BLACK_QUEENS, BLACK_KINGS,
	WHITE_ALL, BLACK_ALL, ALL,
};

typedef struct {
	u64 b[15];
	Player current_player;
} Bitboard;


void BitboardInit(Bitboard *b, const char *fen);
void BitboardMakeMove(Bitboard *b, u8 srck, u8 destk);
bool BitboardIsChecked(Bitboard *b, Player player);
bool BitboardIsMated(Bitboard *b, Player player);
bool BitboardIsDraw(Bitboard *b);
bool BitboardIsValidMove(Bitboard *b, u8 srck, u8 destk, Player player);
void BitboardMakeMoveAlgebraicNotation(Bitboard *b, const char move[4]);
bool BitboardIsValidMoveAlgebraicNotation(Bitboard *b, const char move[4], Player player);
u64 *getBoardByPiece(Bitboard *b, char piece);
u64 AttackMask(Bitboard *b, u8 k, Player player, bool only_attack);

bool isDefendedBy(Bitboard *b, Player player, u8 k);

u64 AttackMaskAllPiece(Bitboard *b, char piece, bool only_attack);
u64 AttackMaskAll(Bitboard *b, Player player, bool only_attack);
bool BitboardCanMate(Bitboard *b, Player player, u8 *srck, u8 *destk);
u8 BitboardCountChecks(Bitboard *b, Player player);

bool BitboardCanPawnPromote(Bitboard *b, Player player, u8 pawnk, u8 *srck, u8 *destk);

void KToAlgebraicNotation(char move[4], u8 srck, u8 destk);

bool getAllValidMoves(Bitboard *b, u8 moves[][2], u8 *len, u8 n, Player player);
void AlgebraicNotationToK(const char move[4], u8 *srck, u8 *destk);
void KToAlgebraicNotation(char move[4], u8 srck, u8 destk);

bool BitboardIsFork(Bitboard *b, Player player, u8 piece);


bool BitboardCanCheck(Bitboard *b, Player player, u8 *srck, u8 *destk);
void BitboardPrint(u64 b);
void BitboardPrintAll(Bitboard *b);

char pieceByBit(Bitboard *b, u8 k);
u8 bsb(u64 n);
u8 bsf(u64 n);
u8 bsf_pop(u64 *n);
u8 count_bits(u64 n);

#endif
