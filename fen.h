/* 
 * this file is used to build fen string for chess
 * you can append all the pieces and get the formated
 * fen as a result
 *
 * example code:
 * ---------------------------
 * FenBuilder fb;
 * FenBuilderInit(&fb);
 * FenBuilderAppendPiece(&fb, 'P');
 * FenBuilderAppendPiece(&fb, 'R');
 * FenBuilderAppendPiece(&fb, EMPTY_SQUARE);
 * FenBuilderAppendPiece(&fb, EMPTY_SQUARE);
 * FenBuilderAppendPiece(&fb, EMPTY_SQUARE);
 * FenBuilderAppendPiece(&fb, EMPTY_SQUARE);
 *
 * // must set player at the end
 * FenBuilderAppendPlayer(&fb, 'w');
 * 
 * printf("FEN: %s\n", FenBuilderToString());
 */

#ifndef FEN_H
#define FEN_H

#include "types_and_macros.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef struct {
	char data[100];
	u8 len;
	u8 row_counter;
} FenBuilder;

#define EMPTY_SQUARE '-'
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w"

void FenBuilderInit(FenBuilder *fb);
void FenBuilderAppendPlayer(FenBuilder *fb, char player);
void FenBuilderAppendPiece(FenBuilder *fb, char player);
char *FenBuilderToString(FenBuilder *fb);
void FenUncompress(char *dest, const char *fen, char *player);
bool FenIsValidFen(const char *fen);

#endif
