#ifndef ENGINE_H
#define ENGINE_H

#include <string.h>


// Fen notation functions
//-------------------------
bool is_legal_fen(const char *fen, int board_dim);


// IMPLEMENTATION
//-------------------------
bool is_legal_fen(const char *fen, int board_dim)
{
	int i = 0;
	int slash = 0;

	while (*fen && *fen != ' ') {
		if (isdigit(*fen))
			i += *fen - '0';
		else if (*fen == '/')
			slash++;
		else if (strchr("rnbqkp", tolower(*fen)))
			i++;
		else 
			return false;
		fen++;
	}
	
	if (!(*(fen++)) || i != board_dim*board_dim || slash != board_dim - 1)
		return false;

	if (!strchr("wb", *(fen++)))
		return false;

	if (!(*(fen++)) && !(*fen + 1))
		return false;

	for (int j = 0; j < 4; j++)
		if (!strchr("-KQkq", *(fen++)))
			return false;

	if (*fen != '\0' && *fen != ' ')
		return false;

	return true;
}

#endif
