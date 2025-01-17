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

static const Bitboard start_bitboard = {
	.current_player = WHITE_PLR,
	.b = {
		0XFF000000000000ULL, 0X4200000000000000ULL, 0X2400000000000000ULL,
		0X8100000000000000ULL, 0X800000000000000ULL, 0X1000000000000000ULL,
		0XFF00ULL, 0X42ULL, 0X24ULL, 0X81ULL, 0X8ULL, 0X10ULL,
		0XFFFF000000000000ULL, 0XFFFFULL, 0XFFFF00000000FFFFULL,
	},
};



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
static inline void loadFen(Bitboard *b, const char *fen);
static inline void setPiece(Bitboard *b, char piece, u8 k, bool val);
bool getAllValidMoves(Bitboard *b, u8 moves[][2], u8 *len, u8 n, Player player);

// fen can be NULL
void BitboardInit(Bitboard *b, const char *fen)
{
	if (fen != NULL)
		loadFen(b, fen);
	else
		*b = start_bitboard;
}

void BitboardMakeMoveAlgebraicNotation(Bitboard *b, const char move[4])
{
	u8 srck;
	u8 destk;
	AlgebraicNotationToK(move, &srck, &destk);
	BitboardMakeMove(b, srck, destk);
}

void BitboardMakeMove(Bitboard *b, u8 srck, u8 destk)
{
	char pb = pieceByBit(b, srck);
	char pa = pieceByBit(b, destk);
	if (pa != -1)
		setPiece(b, pa, destk, 0);
	setPiece(b, pb, srck, 0);
	setPiece(b, pb, destk, 1);

	if (tolower(pb) == 'p' && !IN_BOUNDRY(destk, 7, 56)) {
		setPiece(b, pb, destk, 0);
		setPiece(b, PIECE_COLOR(pb) ? 'Q' : 'q', destk, 1);
	}

	b->current_player = !b->current_player;
}

static inline void setPiece(Bitboard *b, char piece, u8 k, bool val)
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
static inline void loadFen(Bitboard *b, const char *fen)
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
		cnt++;
		k++;
	}

	b->current_player = (player == 'w' ? WHITE_PLR : BLACK_PLR);
}

u8 bsf(u64 n)
{
	if (n == 0)
		return 0;
	return (__builtin_clzll(n) ^ 63) + 1;
}

u64 AttackMaskPawn(Bitboard *b, u8 k, Player player, bool only_attack)
{
	u64 mask = 0;
	mask |= (player ? white_pawn_eat_moves[k+1] : black_pawn_eat_moves[k+1]);
	if (only_attack == false) {
		mask &= b->b[player ? BLACK_ALL : WHITE_ALL];
		mask |= ((player ? white_pawn_moves[k+1] : black_pawn_moves[k+1]) & ~b->b[ALL]);
	}

	return mask;
}

u64 AttackMaskKnight(Bitboard *b, u8 k, Player player)
{
	return knight_moves[k+1] & ~(b->b[player ? WHITE_ALL : BLACK_ALL]);
}

u64 AttackMaskBishop(Bitboard *b, u8 k, Player player)
{
	u64 mask = 0;
	mask |= (~bishop_moves_north_east[bsf(b->b[ALL] & bishop_moves_north_east[k+1])]) & bishop_moves_north_east[k+1];
	mask |= (~bishop_moves_north_west[bsf(b->b[ALL] & bishop_moves_north_west[k+1])]) & bishop_moves_north_west[k+1];
	mask |= (~bishop_moves_south_east[bsb(b->b[ALL] & bishop_moves_south_east[k+1])]) & bishop_moves_south_east[k+1];
	mask |= (~bishop_moves_south_west[bsb(b->b[ALL] & bishop_moves_south_west[k+1])]) & bishop_moves_south_west[k+1];

	mask &= ~(b->b[player ? WHITE_ALL : BLACK_ALL]);

	return mask;
}

u64 AttackMaskRook(Bitboard *b, u8 k, Player player)
{
	u64 mask = 0;
	mask |= (~rook_moves_east[bsb(b->b[ALL] & rook_moves_east[k+1])]) & rook_moves_east[k+1];
	mask |= (~rook_moves_west[bsf(b->b[ALL] & rook_moves_west[k+1])]) & rook_moves_west[k+1];
	mask |= (~rook_moves_north[bsf(b->b[ALL] & rook_moves_north[k+1])]) & rook_moves_north[k+1];
	mask |= (~rook_moves_south[bsb(b->b[ALL] & rook_moves_south[k+1])]) & rook_moves_south[k+1];

	mask &= ~(b->b[player ? WHITE_ALL : BLACK_ALL]);

	return mask;
}

u64 AttackMaskQueen(Bitboard *b, u8 k, Player player)
{
	return AttackMaskBishop(b, k, player) | AttackMaskRook(b, k, player);
}

u64 AttackMaskKing(Bitboard *b, u8 k, Player player)
{
	return king_moves[k+1] & ~(b->b[player ? WHITE_ALL : BLACK_ALL]);
}

u64 AttackMask(Bitboard *b, u8 k, Player player, bool only_attack)
{
	char piece = pieceByBit(b, k);
	switch (piece) {
		case 'P': return AttackMaskPawn(b, k, WHITE_PLR, only_attack);
		case 'p': return AttackMaskPawn(b, k, BLACK_PLR, only_attack);
		case 'n': case 'N': return AttackMaskKnight(b, k, player);
		case 'b': case 'B': return AttackMaskBishop(b, k, player);
		case 'r': case 'R': return AttackMaskRook(b, k, player);
		case 'q': case 'Q': return AttackMaskQueen(b, k, player);
		case 'k': case 'K': return AttackMaskKing(b, k, player);
		default: return 0;
	}
}

u64 AttackMaskAllPiece(Bitboard *b, char piece, bool only_attack)
{
	bool player = PIECE_COLOR(piece);
	u64 board = *getBoardByPiece(b, piece);
	u64 mask = 0;

	while (board)
		mask |= AttackMask(b, bsf_pop(&board) - 1, player, only_attack);	

	return mask;
}

u64 AttackMaskAll(Bitboard *b, Player player, bool only_attack)
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

	return mask;
}

bool isDefendedBy(Bitboard *b, Player player, u8 k)
{
	u64 attack_mask = AttackMaskAll(b, !player, true);
	return attack_mask & MASK(k);
}

char pieceByBit(Bitboard *b, u8 k)
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

// copy no more than n moves
// return true if there is at least 1 move
bool getAllValidMoves(Bitboard *b, u8 moves[][2], u8 *len, u8 n, Player player)
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
	return *len != 0;
}

bool BitboardIsChecked(Bitboard *b, Player player)
{
	return AttackMaskAll(b, !player, true) & (player ? b->b[WHITE_KINGS] : b->b[BLACK_KINGS]);
}

u8 BitboardCountChecks(Bitboard *b, Player player)
{
	u64 king_mask = *getBoardByPiece(b, player ? 'K' : 'k');
	u8 count = 0;
	Bitboard cpy = *b;
	u8 start = player ? 0 : 6;
	u8 end = start + 6;

	for (u8 i = start; i < end; i++)
		while (cpy.b[i])
			if (AttackMask(b, bsf_pop(&cpy.b[i]) - 1, player, true) & king_mask)
				count++;

	return count;
}

bool BitboardCanMate(Bitboard *b, Player player, u8 *srck, u8 *destk)
{
	u8 moves[MAX_MOVES][2];
	u8 len;
	getAllValidMoves(b, moves, &len, MAX_MOVES, player);

	FOR(i, len) {
		Bitboard tmp = *b;
		BitboardMakeMove(&tmp, moves[i][0], moves[i][1]);
		if (BitboardIsMated(&tmp, !player)) {
			*srck = moves[i][0];
			*destk = moves[i][1];
			return true;
		}
	}
	return false;
}

bool BitboardCanCheck(Bitboard *b, Player player, u8 *srck, u8 *destk)
{
	u8 moves[MAX_MOVES][2];
	u8 len;
	getAllValidMoves(b, moves, &len, MAX_MOVES, player);

	FOR(i, len) {
		Bitboard tmp = *b;
		BitboardMakeMove(&tmp, moves[i][0], moves[i][1]);
		if (BitboardIsChecked(&tmp, !player)) {
			*srck = moves[i][0];
			*destk = moves[i][1];
			return true;
		}
	}
	return false;
}


bool BitboardIsDraw(Bitboard *b)
{
	u8 moves[1][2];
	u8 len;

	if (count_bits(b->b[ALL]) < 3)
		return true;

	// only need to check if there is more then 0 legal moves
	if (!getAllValidMoves(b, moves, &len, 1, WHITE_PLR))
		return false;

	if (!getAllValidMoves(b, moves, &len, 1, BLACK_PLR))
		return true;

	return false;
}

bool BitboardIsMated(Bitboard *b, Player player)
{
	// MAX_MOVES is about the max number of moves a player
	// can posibly have, most of the time around 50
	// https://chess.stackexchange.com/questions/4490/maximum-possible-movement-in-a-turn
	Bitboard tmp;
	u8 moves[MAX_MOVES][2];
	u8 len;

	getAllValidMoves(b, moves, &len, MAX_MOVES, player);

	if (BitboardIsChecked(b, player) == false)
		return false;

	// make every move and see if it solves the check
	FOR(i, len) {
		tmp = *b;	
		BitboardMakeMove(&tmp, moves[i][0], moves[i][1]);
		if (BitboardIsChecked(&tmp, player) == false)
			return false;
	}

	return true;
}

bool BitboardIsValidMoveAlgebraicNotation(Bitboard *b, const char move[4], Player player)
{
	u8 srck;
	u8 destk;
	AlgebraicNotationToK(move, &srck, &destk);
	return BitboardIsValidMove(b, srck, destk, player);
}

bool BitboardIsFork(Bitboard *b, Player player, u8 piece)
{
	u64 opponent_pieces = b->b[player ? BLACK_ALL : WHITE_ALL];
	u64 attack_mask = AttackMask(b, piece, player, true);	
	u64 fork_mask = attack_mask & opponent_pieces;

	if (count_bits(fork_mask) < 2)
		return false;

	while (fork_mask) {
		u8 k = bsf_pop(&fork_mask) - 1;
		if (isDefendedBy(b, !player, k))
			return false;
	}

	return true;
}

bool BitboardCanPawnPromote(Bitboard *b, Player player, u8 pawnk, u8 *srck, u8 *destk)
{
	u64 rank_mask = 0XFFULL | (0XFFULL << 56);
	u64 move_mask = AttackMask(b, pawnk, player, false);

	if (rank_mask & move_mask) {
		*srck = pawnk;
		*destk = bsf(move_mask);
	}

	return false;
}

bool BitboardIsValidMove(Bitboard *b, u8 srck, u8 destk, Player player)
{
	Bitboard tmp = *b;
	bool isvalid;
	u64 move_mask;
	u64 after = 0;

	if (PIECE_COLOR(pieceByBit(b, srck)) != player)
		return false;

	TURN_BIT_ON(after, destk);
	move_mask = AttackMask(b, srck, player, false);

	isvalid = move_mask & after & (~b->b[player ? WHITE_ALL : BLACK_ALL]);

	// checking to see if the piece can even move there before checking for check
	if (isvalid == false)
		return false;

	BitboardMakeMove(&tmp, srck, destk);
	if (BitboardIsChecked(&tmp, player))
		return false;

	return true;
}

u8 bsb(u64 n)
{
	if (n == 0)
		return 0;
	return __builtin_ctzll(n) + 1;
}

u8 bsf_pop(u64 *n)
{
	u8 k = bsf(*n);
	TURN_BIT_OFF(*n, k - 1);
	return k;
}

u8 count_bits(u64 n)
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

void KToAlgebraicNotation(char move[4], u8 srck, u8 destk)
{
	move[0]	= '8' - (srck / 8);
	move[1]	= 'a' + (srck % 8);
	move[2]	= '8' - (destk / 8);
	move[3]	= 'a' + (destk % 8);
}

void AlgebraicNotationToK(const char move[4], u8 *srck, u8 *destk)
{
	*srck    = ('8' - move[0]) * 8;
	*srck   +=  move[1] - 'a';
	*destk   = ('8' - move[2]) * 8;
	*destk  +=  move[3] - 'a';
}
