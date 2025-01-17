#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dict.h"

#ifndef GRAPH_TYPE
#define GRAPH_TYPE void*
#endif

typedef struct {
	GRAPH_TYPE info;
	char *name;
} _vertex_t;

typedef struct {
	_vertex_t *vertexs;
	dict_t dict;
	bool **edges;
	int size;
} Graph;

// CORE FUNCTIONS
//--------------------------------------------------------------
void GraphInit(Graph *g);
void GraphAddVertex(Graph *g, char *name, GRAPH_TYPE info);
void GraphAddEdge(Graph *g, const char *v1, const char *v2);
void GraphAddDouble_Edge(Graph *g, const char *v1, const char *v2);
void GraphRemoveEdge(Graph *g, const char *v1, const char *v2);
bool GraphIsAdjacent(Graph *g, const char *v1, const char *v2);
const _vertex_t *Graph_Get_Adjacent_N(Graph *g, const char *v, int n);
void GraphDestroy(Graph *g, void (*free_info)(void *));
void GraphForEveryAdjucent(Graph *g, const char *v, void (*action)(const _vertex_t *v));
bool GraphIsLeaf(Graph *g, const char *v);
GRAPH_TYPE GraphGetVertex(Graph *g, const char *v);

#define FOR_EVERY_ADJUCENT(g, v, adj_name, adj_info, code) \
do { \
	for (int i = 0; i < g->size; i++) { \
		if (GraphIsAdjacent(g, v, g->vertexs[i].name)) { \
			char *adj_name = g->vertexs[i].name; \
			GRAPH_TYPE adj_info = g->vertexs[i].info; \
			do { code } while(0); \
		} \
	} \
} while(0) \

#define FOR_EVERY_ADJUCENT_REVERSE(g, v, adj_name, adj_info, code) \
do { \
	for (int i = g->size - 1; i >= 0; i--) { \
		if (GraphIsAdjacent(g, v, g->vertexs[i].name)) { \
			char *adj_name = g->vertexs[i].name; \
			GRAPH_TYPE adj_info = g->vertexs[i].info; \
			do { code } while(0); \
		} \
	} \
} while(0) \

#endif

/*
     *       *       *       *       *       *       *       *

         *       *       *       *       *       *       *

     *       *       *       *       *       *       *       *
*/
