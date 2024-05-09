#ifndef GENERATE_CONSTANTS_H
#define GENERATE_CONSTANTS_H

#include "types_and_macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TURN_BIT_ON(num, k) 	((num) |= (1ULL << (k)))
#define TURN_BIT_OFF(num, k) 	((num) &= (~(1ULL << (k))))
#define GET_BIT(num, k) 	((bool)(num & (1ULL << (k))))

void printMask(u64 b);
// move
u64 GenerateKingMove(u8 k);
u64 GenerateKnightMove(u8 k);
u64 GeneratePawnMove(u8 k, bool player);
u64 GeneratePawnEatMove(u8 k, bool player);
u64 GenerateRookMoveNorth(u8 k);
u64 GenerateRookMoveSouth(u8 k);
u64 GenerateRookMoveEast(u8 k);
u64 GenerateRookMoveWest(u8 k);
u64 GenerateBishopMoveNorthWest(u8 k);
u64 GenerateBishopMoveSouthWest(u8 k);
u64 GenerateBishopMoveSouthEast(u8 k);
u64 GenerateBishopMoveNorthEast(u8 k);

// moves
void GeneratePawnMoves(u64 moves[65], bool player);
void GeneratePawnEatMoves(u64 moves[65], bool player);
void GenerateKnightMoves(u64 moves[65]);
void GenerateKingMoves(u64 moves[65]);
void GenerateRookMovesNorth(u64 moves[65]);
void GenerateRookMovesSouth(u64 moves[65]);
void GenerateRookMovesWest(u64 moves[65]);
void GenerateRookMovesEast(u64 moves[65]);
void GenerateBishopMovesNorthEast(u64 moves[65]);
void GenerateBishopMovesNorthWest(u64 moves[65]);
void GenerateBishopMovesSouthWest(u64 moves[65]);
void GenerateBishopMovesSouthEast(u64 moves[65]);

void ExportVectorToFile(u64 *vector, u64 len, FILE *file, const char *name, const char *type, u8 values_per_line);

int main(void)
{
	u64 m[65];
	FILE *fp;
	ASSERT(!(fp = fopen("move_set.h", "w")), "generate_move_set.c :: cannot access \"move_set.h\" file");
	fprintf(fp, "#ifndef MOVE_SET_H\n");
	fprintf(fp, "#define MOVE_SET_H\n");

	// king
	memset(m, 0, 65*sizeof(u64));
	GenerateKingMoves(m);
	ExportVectorToFile(m, 65, fp, "king_moves", "static const u64", 4);

	// knight
	memset(m, 0, 65*sizeof(u64));
	GenerateKnightMoves(m);
	ExportVectorToFile(m, 65, fp, "knight_moves", "static const u64", 4);

	// white pawn
	memset(m, 0, 65*sizeof(u64));
	GeneratePawnMoves(m, true);
	ExportVectorToFile(m, 65, fp, "white_pawn_moves", "static const u64", 4);
	
	// black pawn
	memset(m, 0, 65*sizeof(u64));
	GeneratePawnMoves(m, false);
	ExportVectorToFile(m, 65, fp, "black_pawn_moves", "static const u64", 4);

	// white pawn eat
	memset(m, 0, 65*sizeof(u64));
	GeneratePawnEatMoves(m, true);
	ExportVectorToFile(m, 65, fp, "white_pawn_eat_moves", "static const u64", 4);
	
	// black pawn eat
	memset(m, 0, 65*sizeof(u64));
	GeneratePawnEatMoves(m, false);
	ExportVectorToFile(m, 65, fp, "black_pawn_eat_moves", "static const u64", 4);

	// bishop
	memset(m, 0, 65*sizeof(u64));
	GenerateBishopMovesNorthEast(m);
	ExportVectorToFile(m, 65, fp, "bishop_moves_north_east", "static const u64", 4);
	memset(m, 0, 65*sizeof(u64));
	GenerateBishopMovesNorthWest(m);
	ExportVectorToFile(m, 65, fp, "bishop_moves_north_west", "static const u64", 4);
	memset(m, 0, 65*sizeof(u64));
	GenerateBishopMovesSouthEast(m);
	ExportVectorToFile(m, 65, fp, "bishop_moves_south_east", "static const u64", 4);
	memset(m, 0, 65*sizeof(u64));
	GenerateBishopMovesSouthWest(m);
	ExportVectorToFile(m, 65, fp, "bishop_moves_south_west", "static const u64", 4);

	// rook
	memset(m, 0, 65*sizeof(u64));
	GenerateRookMovesNorth(m);
	ExportVectorToFile(m, 65, fp, "rook_moves_north", "static const u64", 4);
	memset(m, 0, 65*sizeof(u64));
	GenerateRookMovesSouth(m);
	ExportVectorToFile(m, 65, fp, "rook_moves_south", "static const u64", 4);
	memset(m, 0, 65*sizeof(u64));
	GenerateRookMovesWest(m);
	ExportVectorToFile(m, 65, fp, "rook_moves_west", "static const u64", 4);
	memset(m, 0, 65*sizeof(u64));
	GenerateRookMovesEast(m);
	ExportVectorToFile(m, 65, fp, "rook_moves_east", "static const u64", 4);

	fprintf(fp, "#endif\n");
	fclose(fp);

	puts("succsesfuly generated \"move_set.h\" file");

	return 0;
}

void ExportVectorToFile(u64 *vector, u64 len, FILE *file, const char *name, const char *type, u8 values_per_line)
{
	fprintf(file, "%s %s[%ld] = {\n", type, name, len);
	fputc('\t', file);
	FOR(i, len) {
		fprintf(file, "0X%lxULL, ", vector[i]);
		if (i % values_per_line == 0) {
			fputc('\n', file);
			fputc('\t', file);
		}
	}
	fprintf(file, "};\n");
}

void GenerateKingMoves(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateKingMove(i);
}

void GenerateKnightMoves(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateKnightMove(i);
}

void GeneratePawnMoves(u64 moves[65], bool player)
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GeneratePawnMove(i, player);
}

void GeneratePawnEatMoves(u64 moves[65], bool player)
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GeneratePawnEatMove(i, player);
}


void GenerateRookMovesNorth(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateRookMoveNorth(i);
}

void GenerateRookMovesSouth(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateRookMoveSouth(i);
}

void GenerateRookMovesWest(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateRookMoveWest(i);
}
void GenerateRookMovesEast(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateRookMoveEast(i);
}

void GenerateBishopMovesNorthEast(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateBishopMoveNorthEast(i);
}

void GenerateBishopMovesSouthEast(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateBishopMoveSouthEast(i);
}

void GenerateBishopMovesNorthWest(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateBishopMoveNorthWest(i);
}

void GenerateBishopMovesSouthWest(u64 moves[65])
{
	moves[0] = 0;
	FOR(i, 64)
		moves[i+1] = GenerateBishopMoveSouthWest(i);
}

u64 GenerateKingMove(u8 k)
{
	u64 base = 0;
	u64 mask = 0;
	i8 cols = k%8;
	TURN_BIT_ON(base, k);

	if (cols < 7)
		mask |= (base << 1) | (base >> 7) | (base << 9);
	if (cols > 0)
		mask |= (base >> 1) | (base << 7) | (base >> 9);

	mask |= (base >> 8) | (base << 8);

	return mask;
}

u64 GenerateKnightMove(u8 k)
{
	u64 base = 0;
	u64 mask = 0;
	i8 cols = k%8;
	TURN_BIT_ON(base, k);

	if (cols < 7) mask |= (base << 17);
	if (cols > 0) mask |= (base >> 17);

	if (cols < 6) mask |= (base << 10);
	if (cols > 1) mask |= (base >> 10);

	if (cols < 7) mask |= (base >> 15);
	if (cols > 0) mask |= (base << 15);

	if (cols < 6) mask |= (base >> 6);
	if (cols > 1) mask |= (base << 6);

	return mask;
}

u64 GeneratePawnMove(u8 k, bool player)
{
	u64 base = 0;
	u64 mask = 0;
	i8 rows = k/8;
	TURN_BIT_ON(base, k);

	if (player && rows < 7) {
		mask |= (base >> 8);
		if (rows == 6)
			mask |= (base >> 16);
	} else if (rows > 0) {
		mask |= (base << 8);
		if (rows == 1)
			mask |= (base << 16);
	}
	return mask;
}

u64 GeneratePawnEatMove(u8 k, bool player)
{
	u64 base = 0;
	u64 mask = 0;
	i8 cols = k%8;
	TURN_BIT_ON(base, k);

	if (player) {
		if (cols > 0)
			mask |= base >> 9;
		if (cols < 7)
			mask |= base >> 7;
	} else {
		if (cols > 0)
			mask |= base << 7;
		if (cols < 7)
			mask |= base << 9;
	}
	return mask;
}

u64 GenerateRookMoveNorth(u8 k)
{
	u64 mask = 0;
	i8 rows = k/8;
	TURN_BIT_ON(mask, k);

	while (rows > 0) {
		mask |= mask >> 8;
		rows--;
	}

	TURN_BIT_OFF(mask, k);
	return mask;
}

u64 GenerateRookMoveSouth(u8 k)
{
	u64 mask = 0;
	i8 rows = k/8;
	TURN_BIT_ON(mask, k);


	while (rows < 8) {
		mask |= mask << 8;
		rows++;
	}

	TURN_BIT_OFF(mask, k);
	return mask;
}

u64 GenerateRookMoveWest(u8 k)
{
	u64 mask = 0;
	i8 cols = k%8;
	TURN_BIT_ON(mask, k);


	while (cols > 0) {
		mask |= mask >> 1;
		cols--;
	}

	TURN_BIT_OFF(mask, k);
	return mask;
}

u64 GenerateRookMoveEast(u8 k)
{
	u64 mask = 0;
	i8 cols = k%8;
	TURN_BIT_ON(mask, k);


	while (cols < 7) {
		mask |= mask << 1;
		cols++;
	}

	TURN_BIT_OFF(mask, k);
	return mask;
}

u64 GenerateBishopMoveNorthEast(u8 k)
{
	u64 mask = 0;
	i8 rows = k/8;
	i8 cols = k%8;
	TURN_BIT_ON(mask, k);


	while (rows > 0 && cols < 7) {
		mask |= mask >> 7;
		rows--;
		cols++;
	}

	TURN_BIT_OFF(mask, k);
	return mask;
}

u64 GenerateBishopMoveSouthEast(u8 k)
{
	u64 mask = 0;
	i8 rows = k/8;
	i8 cols = k%8;
	TURN_BIT_ON(mask, k);


	while (rows < 7 && cols < 7) {
		mask |= mask << 9;
		rows++;
		cols++;
	}

	TURN_BIT_OFF(mask, k);
	return mask;
}

u64 GenerateBishopMoveSouthWest(u8 k)
{
	u64 mask = 0;
	i8 rows = k/8;
	i8 cols = k%8;
	TURN_BIT_ON(mask, k);


	while (rows < 7 && cols > 0) {
		mask |= mask << 7;
		rows++;
		cols--;
	}

	TURN_BIT_OFF(mask, k);
	return mask;
}

u64 GenerateBishopMoveNorthWest(u8 k)
{
	u64 mask = 0;
	i8 rows = k/8;
	i8 cols = k%8;
	TURN_BIT_ON(mask, k);


	while (rows > 0 && cols > 0) {
		mask |= mask >> 9;
		rows--;
		cols--;
	}

	TURN_BIT_OFF(mask, k);
	return mask;
}


void printMask(u64 b)
{
	FOR(i, 8) {
		FOR(j, 8) {
			printf("%2d", GET_BIT(b, i*8 + j));
		}
		putchar('\n');
	}
}

#endif
