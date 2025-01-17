#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ctype.h"
#include "data_structers/dict.h"
#include "engine.h"
#include "bitboard.h"
#include "gui.h"
#include "types_and_macros.h"
#include "data_structers/graph.h"
#include "data_structers/stack.h"
#include "gui.h"

#define START_VERTEX "START"

#define MAX_PIECE_VALUE 10

// TODO: pinning

bool worthEating(Bitboard *b, Player player, u8 srck, u8 destk);
bool canEatPiece(Bitboard *b, Player player, u8 piece, u8 *srck, u8 *destk);

typedef struct {
	bool (*func)(Bitboard*, Player);
} Vertex;


Vertex *mkVertex(bool (*func)(Bitboard*, Player))
{
	Vertex *v = malloc(sizeof(Vertex));
	v->func = func;
	return v;
}

void makeMove(Bitboard *b, u8 srck, u8 destk);

bool alwaysTrue(Bitboard *b, Player player)
{
	return true;
}

bool isCheck(Bitboard *b, Player player)
{
	return BitboardIsChecked(b, player);	
}

// O(player pawns)
bool balancePawns(Bitboard *b, Player player)
{
	u64 pawns = *getBoardByPiece(b, PIECE_BY_PLAYER(player, 'p'));
	u64 pawns_attack = AttackMaskAllPiece(b, PIECE_BY_PLAYER(player, 'p'), true);

	while (pawns) {
		u8 pawn = bsf_pop(&pawns) - 1;
		u64 move = AttackMask(b, pawn, player, false);

		if (move & pawns_attack) {
			makeMove(b, pawn, bsf(move & pawns_attack) - 1);
			return true;
		}
	}
	return false;
}

// O(opponent pieces)
u8 getCheckThreat(Bitboard *b, Player player)
{
	u64 king_mask = *getBoardByPiece(b, PIECE_BY_PLAYER(player, 'k'));
	u64 opponent = b->b[player ? BLACK_ALL : WHITE_ALL];

	while (opponent) {
		u64 piece = bsf_pop(&opponent) - 1;
		if (king_mask & AttackMask(b, piece, !player, true))
			return piece;
	}
	return NULL_PIECE;
}

// O(1)
u8 pieceValue(char piece)
{
	switch (tolower(piece)) {
		case 'p': return 1;
		case 'n': return 3;
		case 'b': return 3;
		case 'r': return 5;
		case 'q': return 9;
	}
	return 0;
}

// O(pieces)
u8 squareBalance(Bitboard *b, Player player, u8 k)
{
	u64 pieces = b->b[player ? WHITE_ALL : BLACK_ALL];
	u64 mask = MASK(k);
	u8 balance = 0;

	while (pieces) {
		u8 piece = bsf_pop(&pieces);

		if (AttackMask(b, piece, player, true) & mask)
			balance += pieceValue(pieceByBit(b, piece));
	}
	return balance;
}

// O(pieces)
i8 squareBalanceForPlayer(Bitboard *b, Player player, u8 k)
{
	u8 opponent_balance = squareBalance(b, !player, k);
	u8 player_balance = squareBalance(b, player, k);

	return player_balance - opponent_balance;
}


// O(1)
bool isThreat(Bitboard *b, Player player)
{
	u64 opponent_attack = AttackMaskAll(b, !player, true);
	u64 player_pieces = b->b[player ? WHITE_ALL : BLACK_ALL];

	if (opponent_attack & player_pieces)
		puts("YES::::::::::::");
	else
		puts("NO::::::::");

	return opponent_attack & player_pieces;
}

// O(opponent pieces)
u8 getThreat(Bitboard *b, Player player)
{
	u64 player_mask = b->b[player ? WHITE_ALL : BLACK_ALL];
	u64 opponent = b->b[player ? BLACK_ALL : WHITE_ALL];

	while (opponent) {
		u8 piece = bsf_pop(&opponent) - 1;
		u8 piece_value = pieceValue(pieceByBit(b, piece));
		u8 attack = AttackMask(b, piece, !player, true);

		if ((attack & player_mask) && (piece_value > bsf(player_mask & attack) - 1))
			return piece;
	}
	return NULL_PIECE;
}

// O(1)
bool isQueenGetsExposed(Bitboard *b, Player player, u8 srck, u8 destk)
{
	Bitboard after = *b;
	BitboardMakeMove(&after, srck, destk);

	u64 queen_mask = *getBoardByPiece(&after, PIECE_BY_PLAYER(player, 'q'));
	u64 opponent_attack = AttackMaskAll(&after, !player, true);

	if (opponent_attack & queen_mask)
		return true;

	return false;
}

bool isSafeMove(Bitboard *b, Player player, u8 srck, u8 destk)
{
	i8 balance = squareBalanceForPlayer(b, player, destk);
	if (balance < 0)
		return false;

	if (isQueenGetsExposed(b, player, srck, destk))
		return false;

	return true;
}

u8 getThreatenPiece(Bitboard *b, Player player)
{
	u64 player_pieces = b->b[player ? WHITE_ALL : BLACK_ALL];
	u8 max_value = 0;
	u8 max_k = NULL_PIECE;

	while (player_pieces) {
		u8 k = bsf_pop(&player_pieces) - 1;
		char piece = pieceByBit(b, k);
		u8 srck;
		u8 destk;

		if (piece == -1)
			continue;

		if (!canEatPiece(b, !player, k, &srck, &destk))
			continue;

		if (worthEating(b, !player, srck, destk) && pieceValue(piece) > max_value) {
			max_k = k;
			max_value = pieceValue(piece);
		}
	}

	puts("-------------------");
	printf("%d\n", max_k);

	return max_k;
}

// O(pieces)
bool runPiece(Bitboard *b, Player player, u8 piece)
{
	u8 srck = piece;
	u64 move_mask = AttackMask(b, srck, player, false);
	u64 opponent_mask = AttackMaskAll(b, !player, true);

	// no place to run to
	if (!((~opponent_mask) & move_mask))
		return false;

	u64 run_mask = (~opponent_mask) & move_mask;
	while (run_mask) {
		u8 destk = bsf_pop(&run_mask);
		if (isSafeMove(b, player, srck, destk) && BitboardIsValidMove(b, srck, destk, player)) {
			makeMove(b, srck, destk);
			return true;
		}
	}
	return false;
}

// O(pieces)
bool run(Bitboard *b, Player player)
{
	u8 srck = getThreatenPiece(b, player);

	if (srck == NULL_PIECE)
		return false;

	return runPiece(b, player, srck);
}

// O(valid moves)
bool canFork(Bitboard *b, Player player, u8 *srck, u8 *destk)
{
	u8 moves[MAX_MOVES][2];
	u8 len;
	getAllValidMoves(b, moves, &len, MAX_MOVES, player);

	for (int i = 0; i < len; i++) {
		Bitboard tmp = *b;
		BitboardMakeMove(&tmp, moves[i][0], moves[i][1]);
		if (BitboardIsFork(&tmp, player, moves[i][1])) {
			*srck = moves[i][0];
			*destk = moves[i][1];
			return true;
		}
	}
	return false;
}

// O(valid moves)
bool Fork(Bitboard *b, Player player)
{
	u8 srck;
	u8 destk;
	u64 opponent_attack = AttackMaskAll(b, !player, true);
	if (canFork(b, player, &srck, &destk) && (opponent_attack & MASK(destk)) == 0) {
		makeMove(b, srck, destk);
		return true;
	}
	return false;
}

// O(player pawns)
bool promotePawn(Bitboard *b, Player player)
{
	u64 pawns = *getBoardByPiece(b, PIECE_BY_PLAYER(player, 'p'));
	u8 srck;
	u8 destk;

	while (pawns) {
		u8 pawn = bsf_pop(&pawns) - 1;
		if (BitboardCanPawnPromote(b, player, pawn, &srck, &destk) && isSafeMove(b, player, srck, destk)) {
			makeMove(b, srck, destk);	
			return true;
		}
	}

	return false;
}

// O(1)
bool moveKing(Bitboard *b, Player player)
{
	u8 king = bsf(*getBoardByPiece(b, PIECE_BY_PLAYER(player, 'k')));
	u64 king_moves = AttackMask(b, king, player, false);
	u8 destk = bsf_pop(&king_moves);

	makeMove(b, king, destk);
	return true;
}

bool canEatPiece(Bitboard *b, Player player, u8 piece, u8 *srck, u8 *destk)
{
	u8 moves[MAX_MOVES][2];
	u8 len;
	getAllValidMoves(b, moves, &len, MAX_MOVES, player);

	u8 p = NULL_PIECE;
	u8 min_value = MAX_PIECE_VALUE;

	for (int i = 0; i < len; i++) {
		u8 piece_value = pieceValue(pieceByBit(b, moves[i][0]));
		if (moves[i][1] == piece && piece_value < min_value) {
			p = moves[i][0];
			min_value = piece_value;
		}
	}

	if (p != NULL_PIECE) {
		*srck = p;
		*destk = piece;
		return true;
	}
	return false;
}

// O(valid moves)
bool eatPiece(Bitboard *b, Player player, u8 threatk)
{
	u8 moves[MAX_MOVES][2];
	u8 len;
	getAllValidMoves(b, moves, &len, MAX_MOVES, player);

	u8 piece = NULL_PIECE;
	u8 min_value = MAX_PIECE_VALUE;

	for (int i = 0; i < len; i++) {
		u8 piece_value = pieceValue(pieceByBit(b, moves[i][0]));
		if (moves[i][1] == threatk && piece_value < min_value) {
			piece = moves[i][0];
			min_value = piece_value;
		}
	}

	if (piece != NULL_PIECE) {
		makeMove(b, piece, threatk);
		return true;
	}
	return false;
}

// O(valid moves)
bool blockCheck(Bitboard *b, Player player)
{
	u8 moves[MAX_MOVES][2];
	u8 len;
	getAllValidMoves(b, moves, &len, MAX_MOVES, player);

	for (int i = 0; i < len; i++) {
		Bitboard tmp = *b;
		BitboardMakeMove(&tmp, moves[i][0], moves[i][1]);
		if (!BitboardIsChecked(&tmp, player)) {
			makeMove(b, moves[i][0], moves[i][1]);
			return true;
		}
	}
	return false;
}

// O(valid moves)
bool solveCheck(Bitboard *b, Player player)
{
	if (BitboardCountChecks(b, player) > 1)
		return moveKing(b, player);

	u8 threat = getCheckThreat(b, player);

	if (eatPiece(b, player, threat))
		return true;

	return blockCheck(b, player);
}

bool isSquareThreatenBy(Bitboard *b, Player player, u8 k)
{
	u8 attack_mask = AttackMaskAll(b, player, true);
	return attack_mask & MASK(k);
}

// O(valid moves)
bool eatThreat(Bitboard *b, Player player)
{
	u8 threatk = getThreat(b, player);

	if (threatk == NULL_PIECE)
		return false;

	u8 srck;
	u8 destk;
	if (canEatPiece(b, player, threatk, &srck, &destk) && !isSquareThreatenBy(b, !player, destk)) {
		makeMove(b, srck, destk);
		return true;
	}
	return false;
}

// O(pieces)
u8 getFork(Bitboard *b, Player player)
{
	u64 player_pieces = b->b[player ? WHITE_ALL : BLACK_ALL];
	while (player_pieces) {
		u8 piece = bsf_pop(&player_pieces);
		if (BitboardIsFork(b, player, piece))
			return piece;
	}
	return NULL_PIECE;
}

// O(1)
void getForkedPieces(Bitboard *b, Player player, u8 fork, u8 *p1, u8 *p2)
{
	u64 attack_mask = AttackMask(b, fork, !player, true);
	u64 mask = attack_mask & b->b[player ? WHITE_ALL : BLACK_ALL];

	*p1 = bsf_pop(&mask);
	*p2 = bsf_pop(&mask);
}

// O(pieces)
bool removeFork(Bitboard *b, Player player)
{
	u8 fork = getFork(b, !player);	
	if (fork == NULL_PIECE)
		return false;

	if (eatPiece(b, player, fork))
		return true;

	u8 p1;
	u8 p2;
	getForkedPieces(b, player, fork, &p1, &p2);

	if (runPiece(b, player, p1) || runPiece(b, player, p2))
		return true;
	return false;
}

// O((valid moves)^2)
bool mate(Bitboard *b, Player player)
{
	u8 srck;
	u8 destk;

	if (!BitboardCanMate(b, player, &srck, &destk))
		return false;
	makeMove(b, srck, destk);
	return true;
}

// O(valid moves)
bool check(Bitboard *b, Player player)
{
	u8 srck;
	u8 destk;

	if (BitboardCanCheck(b, player, &srck, &destk) && squareBalanceForPlayer(b, player, destk) > 0) {
		makeMove(b, srck, destk);
		return true;
	}
	return false;
}

bool worthEating(Bitboard *b, Player player, u8 srck, u8 destk)
{
	char src_piece = pieceByBit(b, srck);
	char dest_piece = pieceByBit(b, destk);

	if (src_piece == -1 || dest_piece == -1)
		return false;

	if (PIECE_COLOR(dest_piece) == player)
		return false;

	if (!isSafeMove(b, player, srck, destk))
		return false;

	if (pieceValue(src_piece) <= pieceValue(dest_piece) || !isDefendedBy(b, !player, destk))
		return true;

	return false;
}

// O(valid moves)
bool eat(Bitboard *b, Player player)
{
	u8 moves[MAX_MOVES][2];
	u8 len;
	getAllValidMoves(b, moves, &len, MAX_MOVES, player);

	for (int i = 0; i < len; i++) {
		if (worthEating(b, player, moves[i][0], moves[i][0])) {
			makeMove(b, moves[i][0], moves[i][1]);
			return true;
		}
	}

	return false;
}

// O(adjucents)
void addAdjucentsToStack(Graph *g, stk_t *s, dict_t *dict, char *v)
{
	FOR_EVERY_ADJUCENT_REVERSE(g, v, adj_name, adj_info,
		if (!dict_find(dict, adj_name))
			stk_push(s, adj_name);
		(void)adj_info;
	);
}

// O(adjucents + edges)
void dfs(Graph *g, Bitboard *b, Player player)
{
	stk_t s;
	stk_init(&s);
	dict_t visited;
	dict_init(&visited, g->size);
	stk_push(&s, START_VERTEX);

	while (!stk_is_empty(&s)) {
		char *v = stk_pop(&s);
		Vertex *vertex = GraphGetVertex(g, v);
		bool res = vertex->func(b, player);

		printf("VERTEX:::::  %s\n", v);

		if (!dict_find(&visited, v) && res) {
			dict_insert(&visited, v, (void*)1);

			if (GraphIsLeaf(g, v))
				return;

			addAdjucentsToStack(g, &s, &visited, v);
		}
	}
}

void playBestMove(Graph *g, Bitboard *b, Player player)
{
	dfs(g, b, player);
}

bool randomMove(Bitboard *b, Player player)
{
	u8 moves[MAX_MOVES][2];
	u8 len;
	getAllValidMoves(b, moves, &len, MAX_MOVES, player);
	u8 i = rand() % len;
	makeMove(b, moves[i][0], moves[i][1]);
	return true;
}

void makeMove(Bitboard *b, u8 srck, u8 destk)
{
	char move[4];
	KToAlgebraicNotation(move, srck, destk);
	BitboardMakeMove(b, srck, destk);
	GuiMakeMove(move);
}

void initStateGraph(Graph *g)
{
	GraphInit(g);

	// start
	GraphAddVertex(g, START_VERTEX, mkVertex(alwaysTrue));

	// check
	GraphAddVertex(g, "check", mkVertex(isCheck));
	GraphAddVertex(g, "solve-check", mkVertex(solveCheck));

	// defence
	GraphAddVertex(g, "defence", mkVertex(isThreat));
	GraphAddVertex(g, "eat-threat", mkVertex(eatThreat));
	GraphAddVertex(g, "remove-fork", mkVertex(removeFork));
	GraphAddVertex(g, "run", mkVertex(run));

	// attack
	GraphAddVertex(g, "attack", mkVertex(alwaysTrue));
	GraphAddVertex(g, "do-mate", mkVertex(mate));
	GraphAddVertex(g, "do-check", mkVertex(check));
	GraphAddVertex(g, "eat", mkVertex(eat));
	GraphAddVertex(g, "do-fork", mkVertex(Fork));

	// develop
	GraphAddVertex(g, "develop", mkVertex(alwaysTrue));
	GraphAddVertex(g, "promote-pawn", mkVertex(promotePawn));
	GraphAddVertex(g, "balance-pawns", mkVertex(balancePawns));

	// safty net
	GraphAddVertex(g, "random-move", mkVertex(randomMove));

	// edges
	GraphAddEdge(g, START_VERTEX, "check");
	GraphAddEdge(g, START_VERTEX, "defence");
	GraphAddEdge(g, START_VERTEX, "attack");
	GraphAddEdge(g, START_VERTEX, "develop");
	GraphAddEdge(g, START_VERTEX, "random-move");
	GraphAddEdge(g, "check", "solve-check");
	GraphAddEdge(g, "defence", "eat-threat");
	GraphAddEdge(g, "defence", "remove-fork");
	GraphAddEdge(g, "defence", "run");
	GraphAddEdge(g, "attack", "do-mate");
	GraphAddEdge(g, "attack", "do-check");
	GraphAddEdge(g, "attack", "eat");
	GraphAddEdge(g, "attack", "do-fork");
	GraphAddEdge(g, "develop", "promote-pawn");
	GraphAddEdge(g, "develop", "balance-pawns");
}
