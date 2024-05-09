#include "fen.h"
#include "types_and_macros.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void FenBuilderInit(FenBuilder *fb)
{
	fb->len = 0;
	fb->row_counter = 0;
}

void FenBuilderAppendPiece(FenBuilder *fb, char piece)
{
	if (fb->row_counter == 8) {
		fb->data[fb->len++] = '/';
		fb->row_counter = 0;
	} 
	if (IS_PIECE(piece))
		fb->data[fb->len++] = piece;
	else if (fb->len && isdigit(fb->data[fb->len - 1]))
		fb->data[fb->len - 1]++;
	else
		fb->data[fb->len++] = '1';
	fb->row_counter++;
}

void FenBuilderAppendPlayer(FenBuilder *fb, char player)
{
	fb->data[fb->len++] = ' ';
	fb->data[fb->len++] = player;
	fb->data[fb->len++] = '\0';
}

char *FenBuilderToString(FenBuilder *fb)
{
	return fb->data;
}

bool FenIsValidFen(const char *fen)
{
	u16 i = 0;
	u16 slash = 0;

	while (*fen && *fen != ' ') {
		if (isdigit(*fen))
			i += *fen - '0';
		else if (*fen == '/')
			slash++;
		else if (IS_PIECE(*fen))
			i++;
		else
			return false;
		fen++;
	}

	if (*fen == '\0') 		return false;
	if (*fen != ' ') 		return false;
	if (!strchr("wb", fen[1])) 	return false;
	if (i != 64) 			return false;
	if (slash != 7) 		return false;

	return true;
}

void FenUncompress(char *dest, const char *fen, char *player)
{
	while (*fen != ' ') {
		if (IS_PIECE(*fen)) {
			*(dest++) = *fen;
		} else if (isdigit(*fen)) {
			FOR(i, *fen - '0') {
				*(dest++) = EMPTY_SQUARE;
			}
		}
		fen++;
	}

	*player = *(++fen);

	*dest = '\0';
}

