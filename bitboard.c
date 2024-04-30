#include "bitboard.h"
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <wh/messure_time.h>
#include "constants.h"
#include "macros.h"

/*
 * TODO
 * check if load fen works
 */

static const char *pieces = "PNBRQKpnbrqk";

// bits
#define TURN_BIT_ON(num, k) 	((num) |= (1ULL << (k)))
#define TURN_BIT_OFF(num, k) 	((num) &= (~(1ULL << (k))))
#define GET_BIT(num, k) 	((bool)(num & (1ULL << (k))))
#define PIECE_COLOR(p) 		((bool)isupper(p))

u8 bsb(u64 n);
u8 bsf(u64 n);
u8 bsf_pop(u64 *n);
u8 bsb_pop(u64 *n);
u8 count_bits(u64 n);

u8 count_bits(u64 n)
{
	u8 i = 0;
	while (n) {
		if (n & 1ULL)
			i++;
		n >>= 1;
	}
	return i;
}

// attack masks
u64 AttackMaskPawn(Bitboard *b, u8 k, Player player, bool only_attack);
u64 AttackMaskBishop(Bitboard *b, u8 k, Player player);
u64 AttackMaskRook(Bitboard *b, u8 k, Player player);
u64 AttackMaskQueen(Bitboard *b, u8 k, Player player);
u64 AttackMaskKing(Bitboard *b, u8 k, Player player);
u64 AttackMask(Bitboard *b, u8 k, Player player, bool only_attack);
u64 AttackMaskAll(Bitboard *b, Player player, bool only_attack);

// misc
u64 *getBoardByPiece(Bitboard *b, char piece);
char pieceByBit(Bitboard *b, u8 k);
void loadFen(Bitboard *b, const char *fen);
void setPiece(Bitboard *b, char piece, u8 k, bool val);
void getAllValidMoves(Bitboard *b, u8 moves[][2], u32 *len, Player player);

// fen can be NULL
void BitboardInit(Bitboard *b, const char *fen)
{
	if (fen != NULL)
		loadFen(b, fen);
	else
		*b = start_bitboard;
}

void BitboardMakeMove(Bitboard *b, u64 srck, u64 destk)
{
	char pb = pieceByBit(b, srck);
	char pa = pieceByBit(b, destk);
	if (pa != -1)
		setPiece(b, pa, destk, 0);
	setPiece(b, pb, srck, 0);
	setPiece(b, pb, destk, 1);

	if (tolower(pb) == 'p' && (destk <= 7 || destk >= 56)) {
		setPiece(b, pb, destk, 0);
		setPiece(b, PIECE_COLOR(pb) ? 'Q' : 'q', destk, 1);
	}

	b->current_player = !b->current_player;
}

void setPiece(Bitboard *b, char piece, u8 k, bool val)
{
	if (val) {
		TURN_BIT_ON(*getBoardByPiece(b, piece), k);
		TURN_BIT_ON(b->b[ALL], k);
		TURN_BIT_ON(b->b[PIECE_COLOR(piece) ? WHITE_ALL : BLACK_ALL], k);
		TURN_BIT_OFF(b->b[PIECE_COLOR(piece) ? BLACK_ALL : WHITE_ALL], k);
	} else {
		TURN_BIT_OFF(*getBoardByPiece(b, piece), k);
		TURN_BIT_OFF(b->b[ALL], k);
		TURN_BIT_OFF(b->b[PIECE_COLOR(piece) ? WHITE_ALL : BLACK_ALL], k);
	}
}

// asumming the fen is legal
void loadFen(Bitboard *b, const char *fen)
{
	u8 cols = 0;
	u8 rows = 0;

	while (*fen != ' ') {
		if (*fen == '/') {
			rows++;
			cols = 0;
		} else if (isdigit(*fen)) {
			cols += (*fen - '0');
		} else {
			/* printf("%c Board: %d\n", *fen, pieceOffset(*fen)); */
			setPiece(b, *fen, rows*8 + cols, 1);
			cols++;
		}
		fen++;
	}

	b->current_player = (*(++fen) == 'w' ? WHITE : BLACK);
}

u8 bsf(u64 n)
{
	if (n == 0)
		return 0;
	return (__builtin_clzll(n) ^ 63) + 1;
}

u8 bsb(u64 n)
{
	if (n == 0)
		return 0;
	return __builtin_ctzll(n) + 1;
}

u8 bsb_pop(u64 *n)
{
	u8 k = bsb(*n);
	TURN_BIT_OFF(*n, k);
	return k;
}

u8 bsf_pop(u64 *n)
{
	u8 k = bsf(*n);
	TURN_BIT_OFF(*n, k - 1);
	return k;
}

u64 AttackMaskPawn(Bitboard *b, u8 k, Player player, bool only_attack)
{
	u64 mask = 0;
	mask |= (player ? white_pawn_eat_moves[k] : black_pawn_eat_moves[k]);
	if (only_attack == false) {
		mask &= b->b[player ? BLACK_ALL : WHITE_ALL];
		mask |= ((player ? white_pawn_moves[k] : black_pawn_moves[k]) & ~b->b[ALL]);
	}
	return mask;
}

u64 AttackMaskKnight(Bitboard *b, u8 k, Player player)
{
	return knight_moves[k] & ~(b->b[player ? WHITE_ALL : BLACK_ALL]);
}

u64 AttackMaskBishop(Bitboard *b, u8 k, Player player)
{
	u64 mask = 0;
	mask |= (~bishop_rays[bsb(b->b[ALL] & bishop_rays[k+1][0])][0]) & bishop_rays[k+1][0]; 
	mask |= (~bishop_rays[bsb(b->b[ALL] & bishop_rays[k+1][1])][1]) & bishop_rays[k+1][1]; 
	mask |= (~bishop_rays[bsf(b->b[ALL] & bishop_rays[k+1][2])][2]) & bishop_rays[k+1][2]; 
	mask |= (~bishop_rays[bsf(b->b[ALL] & bishop_rays[k+1][3])][3]) & bishop_rays[k+1][3]; 

	mask &= ~(b->b[player ? WHITE_ALL : BLACK_ALL]);

	return mask;
}

u64 AttackMaskRook(Bitboard *b, u8 k, Player player)
{
	u64 mask = 0;
	mask |= (~rook_rays[bsb(b->b[ALL] & rook_rays[k+1][0])][0]) & rook_rays[k+1][0];
	mask |= (~rook_rays[bsf(b->b[ALL] & rook_rays[k+1][1])][1]) & rook_rays[k+1][1];
	mask |= (~rook_rays[bsf(b->b[ALL] & rook_rays[k+1][2])][2]) & rook_rays[k+1][2];
	mask |= (~rook_rays[bsb(b->b[ALL] & rook_rays[k+1][3])][3]) & rook_rays[k+1][3];

	mask &= ~(b->b[player ? WHITE_ALL : BLACK_ALL]);

	return mask;
}

u64 AttackMaskQueen(Bitboard *b, u8 k, Player player)
{
	return AttackMaskBishop(b, k, player) | AttackMaskRook(b, k, player);
}

u64 AttackMaskKing(Bitboard *b, u8 k, Player player)
{
	return king_moves[k] & ~(b->b[player ? WHITE_ALL : BLACK_ALL]);
}

u64 AttackMaskAll(Bitboard *b, Player player, bool only_attack)
{
	u64 mask = 0;
	u8 start = player ? 0 : 6;

	Bitboard cpy = *b;

	for (u8 i = start; i < start + 6; i++) {
		while (cpy.b[i]) {
			mask |= AttackMask(b, bsf_pop(&cpy.b[i]) - 1, player, only_attack) & ~(cpy.b[player ? WHITE_ALL : BLACK_ALL]);
		}
	}

	return mask;
}

char pieceByBit(Bitboard *b, u8 k)
{
	FOR(i, 12) {
		if (GET_BIT(b->b[i], k))
			return pieces[i];
	}
	return -1;
}

u64 AttackMask(Bitboard *b, u8 k, Player player, bool only_attack)
{
	char piece = pieceByBit(b, k);
	switch (piece) {
		case 'P': return AttackMaskPawn(b, k, WHITE, only_attack);
		case 'p': return AttackMaskPawn(b, k, BLACK, only_attack);
		case 'n': case 'N': return AttackMaskKnight(b, k, player);
		case 'b': case 'B': return AttackMaskBishop(b, k, player);
		case 'r': case 'R': return AttackMaskRook(b, k, player);
		case 'q': case 'Q': return AttackMaskQueen(b, k, player);
		case 'k': case 'K': return AttackMaskKing(b, k, player);
		default: return 0;
	}
}

u64 *getBoardByPiece(Bitboard *b, char piece)
{
	switch (piece) {
		case 'P': return &b->b[WHITE_PAWNS];
		case 'N': return &b->b[WHITE_KNIGHTS];
		case 'B': return &b->b[WHITE_BISHOPS];
		case 'R': return &b->b[WHITE_ROOKS];
		case 'Q': return &b->b[WHITE_QUEENS];
		case 'K': return &b->b[WHITE_KINGS];
		case 'p': return &b->b[BLACK_PAWNS];
		case 'n': return &b->b[BLACK_KNIGHTS];
		case 'b': return &b->b[BLACK_BISHOPS];
		case 'r': return &b->b[BLACK_ROOKS];
		case 'q': return &b->b[BLACK_QUEENS];
		case 'k': return &b->b[BLACK_KINGS];
		default: return NULL;
	}
}

void getAllValidMoves(Bitboard *b, u8 moves[][2], u32 *len, Player player)
{
	Bitboard cpy = *b;
	u64 mask = 0;
	u8 start = player ? 0 : 6;
	u8 k1, k2;

	*len = 0;

	for (u8 i = start; i < start + 6; i++) {
		while (cpy.b[i]) {
			k1 = bsf_pop(&cpy.b[i]) - 1;
			mask = AttackMask(b, k1, player, false) & ~(cpy.b[player ? WHITE_ALL : BLACK_ALL]);
			while (mask) {
				k2 = bsf_pop(&mask) - 1;
				if (BitboardIsValidMove(b, k1, k2)) {
					moves[*len][0] = k1;
					moves[*len][1] = k2;
					(*len)++;
				}
			}
		}
	}
}

bool BitboardIsChecked(Bitboard *b, Player player)
{
	return AttackMaskAll(b, !player, true) & (player ? b->b[WHITE_KINGS] : b->b[BLACK_KINGS]);
}

/* bool BitboardIsDraw(Bitboard *b) */
/* { */
/* 	if (count_bits(b->b[ALL]) == 2) */
/* 		return true; */
/* } */

/* bool BitboardIsEnd(Bitboard *b, ) */

bool BitboardIsMated(Bitboard *b, Player player)
{
	Bitboard tmp;
	u8 moves[218][2];
	u32 len;

	getAllValidMoves(b, moves, &len, b->current_player);

	if (BitboardIsChecked(b, player) == false)
		return false;

	FOR(i, len) {
		tmp = *b;	
		BitboardMakeMove(&tmp, moves[i][0], moves[i][1]);
		if (BitboardIsChecked(&tmp, player) == false)
			return false;
	}

	return true;
}

bool BitboardIsValidMove(Bitboard *b, u8 srck, u8 destk)
{
	Bitboard tmp = *b;
	u64 move_mask;
	u64 after = 0;

	TURN_BIT_ON(after, destk);

	if (PIECE_COLOR(pieceByBit(b, srck)) != b->current_player)
		return false;
	
	move_mask = AttackMask(b, srck, b->current_player, false);

	if ((after & move_mask) == false)
		return false;

	BitboardMakeMove(&tmp, srck, destk);

	if (BitboardIsChecked(&tmp, b->current_player))
		return false;


	return move_mask & after & (~b->b[b->current_player ? WHITE_ALL : BLACK_ALL]);
}

void BitboardPrintAll(Bitboard *b)
{
	FOR(i, 15) {
		printf("board: %s\n", names[i]);
		BitboardPrint(b->b[i]);
		puts("----------------------");
	}
}

void BitboardPrint(u64 b)
{
	FOR(i, 8) {
		FOR(j, 8) {
			printf("%2d", GET_BIT(b, i*8 + j));
		}
		putchar('\n');
	}
}
