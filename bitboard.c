#include "bitboard.h"
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <wh/messure_time.h>
#include "fen.h"
#include "move_set.h"
#include "types_and_macros.h"

/*
 * TODO
 * check if load fen works
 */

static const Bitboard start_bitboard = {
	.current_player = WHITE,
	.b = {
		0XFF000000000000ULL, 0X4200000000000000ULL, 0X2400000000000000ULL,
		0X8100000000000000ULL, 0X800000000000000ULL, 0X1000000000000000ULL,
		0XFF00ULL, 0X42ULL, 0X24ULL, 0X81ULL, 0X8ULL, 0X10ULL,
		0XFFFF000000000000ULL, 0XFFFFULL, 0XFFFF00000000FFFFULL,
	},
};

#define TURN_BIT_ON(num, k) 	((num) |= (1ULL << (k)))
#define TURN_BIT_OFF(num, k) 	((num) &= (~(1ULL << (k))))
#define MASK(k) 		(1ULL << k)
#define GET_BIT(num, k) 	((bool)(num & (1ULL << (k))))
#define PIECE_COLOR(p) 		((bool)isupper(p))

static u8 bsb(u64 n);
static u8 bsf(u64 n);
static u8 bsf_pop(u64 *n);
static u8 count_bits(u64 n);

// attack masks
static u64 AttackMaskPawn(Bitboard *b, u8 k, Player player, bool only_attack);
static u64 AttackMaskBishop(Bitboard *b, u8 k, Player player);
static u64 AttackMaskRook(Bitboard *b, u8 k, Player player);
static u64 AttackMaskQueen(Bitboard *b, u8 k, Player player);
static u64 AttackMaskKing(Bitboard *b, u8 k, Player player);
static u64 AttackMask(Bitboard *b, u8 k, Player player, bool only_attack);
static u64 AttackMaskAll(Bitboard *b, Player player, bool only_attack);

// misc
static u64 *getBoardByPiece(Bitboard *b, char piece);
static char pieceByBit(Bitboard *b, u8 k);
static void loadFen(Bitboard *b, const char *fen);
static void setPiece(Bitboard *b, char piece, u8 k, bool val);
void getAllValidMoves(Bitboard *b, u8 moves[][2], u32 *len, u32 n, Player player);

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

static void setPiece(Bitboard *b, char piece, u8 k, bool val)
{
	bool color = PIECE_COLOR(piece);
	if (val) {
		TURN_BIT_ON(*getBoardByPiece(b, piece), k);
		TURN_BIT_ON(b->b[ALL], k);
		TURN_BIT_ON(b->b[color ? WHITE_ALL : BLACK_ALL], k);
		TURN_BIT_OFF(b->b[color ? BLACK_ALL : WHITE_ALL], k);
	} else {
		
		TURN_BIT_OFF(*getBoardByPiece(b, piece), k);
		TURN_BIT_OFF(b->b[ALL], k);
		TURN_BIT_OFF(b->b[color ? WHITE_ALL : BLACK_ALL], k);
	}
}

// asumming the fen is valid
static void loadFen(Bitboard *b, const char *fen)
{
	char uncompressed_fen[100];
	char player;

	FenUncompress(uncompressed_fen, fen, &player);

	char *cnt = uncompressed_fen;
	*b = (Bitboard){0};
	u8 k = 0;

	while (*cnt) {
		if (*cnt != EMPTY_SQUARE)
			setPiece(b, *cnt, k, 1);
		k++;
	}

	b->current_player = (player == 'w' ? WHITE : BLACK);
}

static u8 bsf(u64 n)
{
	if (n == 0)
		return 0;
	return (__builtin_clzll(n) ^ 63) + 1;
}

static u64 AttackMaskPawn(Bitboard *b, u8 k, Player player, bool only_attack)
{
	u64 mask = 0;
	mask |= (player ? white_pawn_eat_moves[k+1] : black_pawn_eat_moves[k+1]);
	if (only_attack == false) {
		mask &= b->b[player ? BLACK_ALL : WHITE_ALL];
		mask |= ((player ? white_pawn_moves[k+1] : black_pawn_moves[k+1]) & ~b->b[ALL]);
	}

	return mask;
}

static u64 AttackMaskKnight(Bitboard *b, u8 k, Player player)
{
	return knight_moves[k+1] & ~(b->b[player ? WHITE_ALL : BLACK_ALL]);
}

static u64 AttackMaskBishop(Bitboard *b, u8 k, Player player)
{
	u64 mask = 0;
	mask |= (~bishop_moves_north_east[bsf(b->b[ALL] & bishop_moves_north_east[k+1])]) & bishop_moves_north_east[k+1];
	mask |= (~bishop_moves_north_west[bsf(b->b[ALL] & bishop_moves_north_west[k+1])]) & bishop_moves_north_west[k+1];
	mask |= (~bishop_moves_south_east[bsb(b->b[ALL] & bishop_moves_south_east[k+1])]) & bishop_moves_south_east[k+1];
	mask |= (~bishop_moves_south_west[bsb(b->b[ALL] & bishop_moves_south_west[k+1])]) & bishop_moves_south_west[k+1];

	mask &= ~(b->b[player ? WHITE_ALL : BLACK_ALL]);

	return mask;
}

static u64 AttackMaskRook(Bitboard *b, u8 k, Player player)
{
	u64 mask = 0;
	mask |= (~rook_moves_east[bsb(b->b[ALL] & rook_moves_east[k+1])]) & rook_moves_east[k+1];
	mask |= (~rook_moves_west[bsf(b->b[ALL] & rook_moves_west[k+1])]) & rook_moves_west[k+1];
	mask |= (~rook_moves_north[bsf(b->b[ALL] & rook_moves_north[k+1])]) & rook_moves_north[k+1];
	mask |= (~rook_moves_south[bsf(b->b[ALL] & rook_moves_south[k+1])]) & rook_moves_south[k+1];

	mask &= ~(b->b[player ? WHITE_ALL : BLACK_ALL]);


	return mask;
}

static u64 AttackMaskQueen(Bitboard *b, u8 k, Player player)
{
	return AttackMaskBishop(b, k, player) | AttackMaskRook(b, k, player);
}

static u64 AttackMaskKing(Bitboard *b, u8 k, Player player)
{
	return king_moves[k+1] & ~(b->b[player ? WHITE_ALL : BLACK_ALL]);
}

static u64 AttackMask(Bitboard *b, u8 k, Player player, bool only_attack)
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

static u64 AttackMaskAll(Bitboard *b, Player player, bool only_attack)
{
	Bitboard cpy = *b;
	u64 mask = 0;
	u8 start = player ? 0 : 6;
	u8 end = start + 6;

	for (u8 i = start; i < end; i++) {
		while (cpy.b[i]) {
			mask |= AttackMask(b, bsf_pop(&cpy.b[i]) - 1, player, only_attack);
		}
	}

	// remove all friendly pieces
	mask &= ~(cpy.b[player ? WHITE_ALL : BLACK_ALL]);

	return mask;
}

static char pieceByBit(Bitboard *b, u8 k)
{
	u8 start = 6;
	u64 mask = MASK(k);

	// if there is no such piece return -1
	if (!(b->b[ALL] & mask))
		return -1;

	// if its a white piece, search in 0-5 white bitboards
	// else search in 5-11 black bitboards
	if (b->b[WHITE_ALL] & mask)
		start = 0;

	FOR_RANGE(i, start, start+6, 1) {
		if (b->b[i] & mask)
			return pieces[i];
	}
	return -1;
}


static u64 *getBoardByPiece(Bitboard *b, char piece)
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

// copy no more than n moves
void getAllValidMoves(Bitboard *b, u8 moves[][2], u32 *len, u32 n, Player player)
{
	Bitboard cpy = *b;
	u64 mask = 0;
	u8 start = player ? 0 : 6;
	u8 end = start + 6;
	u8 k1, k2;

	*len = 0;
	for (u8 i = start; i < end && *len < n; i++) {
		while (cpy.b[i] && *len < n) {
			k1 = bsf_pop(&cpy.b[i]) - 1;
			mask = AttackMask(b, k1, player, false) & ~(cpy.b[player ? WHITE_ALL : BLACK_ALL]);
			while (mask && *len < n) {
				k2 = bsf_pop(&mask) - 1;
				if (BitboardIsValidMove(b, k1, k2, player)) {
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

bool BitboardIsDraw(Bitboard *b)
{
	u8 moves[1][2];
	u32 len;

	if (count_bits(b->b[ALL]) < 3)
		return true;

	// only need to check if there is more then 0 legal moves
	getAllValidMoves(b, moves, &len, 1, WHITE);
	if (len == 0)
		return true;

	getAllValidMoves(b, moves, &len, 1, BLACK);
	if (len == 0)
		return true;

	return false;
}

bool BitboardIsMated(Bitboard *b, Player player)
{
	// 218 is about the max number of moves a player
	// can posibly have, most of the time around 50
	// https://chess.stackexchange.com/questions/4490/maximum-possible-movement-in-a-turn
	Bitboard tmp;
	u8 moves[218][2];
	u32 len;

	getAllValidMoves(b, moves, &len, 218, b->current_player);

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

bool BitboardIsValidMove(Bitboard *b, u8 srck, u8 destk, Player player)
{
	Bitboard tmp = *b;
	bool result;
	u64 move_mask;
	u64 after = 0;

	if (PIECE_COLOR(pieceByBit(b, srck)) != player)
		return false;

	TURN_BIT_ON(after, destk);
	move_mask = AttackMask(b, srck, b->current_player, false);

	result = move_mask & after & (~b->b[b->current_player ? WHITE_ALL : BLACK_ALL]); 

	// checking to see if the piece can even move there before checking for check
	if (result == false)
		return false;

	BitboardMakeMove(&tmp, srck, destk);
	if (BitboardIsChecked(&tmp, b->current_player))
		return false;

	return result;
}

static u8 bsb(u64 n)
{
	if (n == 0)
		return 0;
	return __builtin_ctzll(n) + 1;
}

static u8 bsf_pop(u64 *n)
{
	u8 k = bsf(*n);
	TURN_BIT_OFF(*n, k - 1);
	return k;
}

static u8 count_bits(u64 n)
{
	return __builtin_popcountll(n);
}

void BitboardPrintAll(Bitboard *b)
{
	static const char *names[] = {
		"WHITE_PAWNS", "WHITE_KNIGHTS", "WHITE_BISHOPS", "WHITE_ROOKS", "WHITE_QUEENS",
		"WHITE_KINGS", "BLACK_PAWNS", "BLACK_KNIGHTS", "BLACK_BISHOPS", "BLACK_ROOKS",
		"BLACK_QUEEN", "BLACK_KING", "WHITE_ALL", "BLACK_ALL", "ALL",
	};

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
