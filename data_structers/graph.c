#include "graph.h"
#include "dict.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// IMPLEMENTATION
//--------------------------------------------------------------

void GraphInit(Graph *g)
{
	g->vertexs = NULL;
	g->edges = NULL;
	g->size = 0;
	dict_init(&g->dict, 200);
}

void GraphAddVertex(Graph *g, char *name, GRAPH_TYPE info)
{
	g->vertexs = realloc(g->vertexs, sizeof(_vertex_t) * ++g->size);

	g->vertexs[g->size-1].info = info;
	g->vertexs[g->size-1].name = strdup(name);

	dict_insert(&g->dict, name, (void*)((long long)g->size - 1));

	g->edges = realloc(g->edges, sizeof(bool*) * g->size);

	for (int i = 0; i < g->size-1; i++) {
		g->edges[i] = realloc(g->edges[i], sizeof(bool));
		g->edges[i][g->size-1] = 0;
	}

	g->edges[g->size-1] = calloc(sizeof(bool), g->size);
}

void GraphAddEdge(Graph *g, const char *v1, const char *v2)
{
	int v1_ofst = (long long)dict_find(&g->dict, v1);
	int v2_ofst = (long long)dict_find(&g->dict, v2);

	g->edges[v1_ofst][v2_ofst] = true;
}

void GraphAddDoubleEdge(Graph *g, const char *v1, const char *v2)
{
	int v1_ofst = (long long)dict_find(&g->dict, v1);
	int v2_ofst = (long long)dict_find(&g->dict, v2);

	g->edges[v1_ofst][v2_ofst] = true;
	g->edges[v2_ofst][v1_ofst] = true;
}

void GraphRemoveEdge(Graph *g, const char *v1, const char *v2)
{
	int v1_ofst = (long long)dict_find(&g->dict, v1);
	int v2_ofst = (long long)dict_find(&g->dict, v2);

	g->edges[v1_ofst][v2_ofst] = false;
}

bool GraphIsAdjacent(Graph *g, const char *v1, const char *v2)
{
	int v1_ofst = (long long)dict_find(&g->dict, v1);
	int v2_ofst = (long long)dict_find(&g->dict, v2);

	return g->edges[v1_ofst][v2_ofst];
}

const _vertex_t *GraphGetAdjacentN(Graph *g, const char *v, int n)
{
	int v_ofst = (long long)dict_find(&g->dict, v);;

	int count = 0;
	for (int i = 0; i < g->size; i++)
		if (g->edges[v_ofst][i] && count++ == n)
			return &g->vertexs[i];
	return NULL;
}

void GraphDestroy(Graph *g, void (*free_info)(void *))
{
	dict_destroy(&g->dict, NULL);
	if (free_info) {
		for (int i = 0; i < g->size; i++) {
			free_info(g->vertexs[i].info);
		}
	}

	for (int i = 0; i < g->size; i++) {
		free(g->vertexs[i].name);
	}

	free(g->vertexs);
	for (int i = 0; i < g->size; i++) {
		free(g->edges[i]);
	}
	free(g->edges);
}

void GraphForEveryAdjucent(Graph *g, const char *v, void (*action)(const _vertex_t *v))
{
	for (int i = 0; i < g->size; i++) {
		if (GraphIsAdjacent(g, g->vertexs[i].name, v)) {
			action(&g->vertexs[i]);
		}
	}
}

bool GraphIsLeaf(Graph *g, const char *v)
{
	for (int i = 0; i < g->size; i++)
		if (GraphIsAdjacent(g, v, g->vertexs[i].name))
			return false;
	return true;
}

GRAPH_TYPE GraphGetVertex(Graph *g, const char *v)
{
	int v_ofst = (long long)dict_find(&g->dict, v);;
	return g->vertexs[v_ofst].info;
}


/*
     *       *       *       *       *       *       *       *

         *       *       *       *       *       *       *

     *       *       *       *       *       *       *       *
*/

// LOCAL
//--------------------------------------------------------------


void grp_print_edge_mat(Graph *g)
{
	for (int i = 0; i < g->size; i++) {
		for (int j = 0; j < g->size; j++) {
			printf("%2d", g->edges[i][j]);
		}
		putchar('\n');
	}
}
