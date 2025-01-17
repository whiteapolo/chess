#include "fen.h"
#include "types_and_macros.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

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

bool check_regex(const char *reg, const char *str)
{
	regex_t regex;
	regcomp(&regex, reg, 0);
	bool result = !regexec(&regex, str, 0, NULL, 0);
	regfree(&regex);
	return result;
}

bool FenIsValidFen(const char *fen)
{
	return true;
	if (!check_regex("^([1-8PNBRQK]+\\/){7}[1-8PNBRQK]+ [wb]", fen))
		return false;

	int i = 0;
	while (*fen != ' ') {
		if (IS_PIECE(*fen))
			i++;
		else if (isdigit(*fen))
				i += *fen - '0';
		fen++;
	}
	PRINT_VAR(i);
	return i == 64;
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

