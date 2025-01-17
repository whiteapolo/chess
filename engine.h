#ifndef ENGINE_H
#define ENGINE_H

#include "data_structers/graph.h"
#include "bitboard.h"

void initStateGraph(Graph *g);
void playBestMove(Graph *g, Bitboard *b, Player player);

#endif
