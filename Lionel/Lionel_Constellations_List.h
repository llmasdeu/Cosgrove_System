/**
 * SISTEMES OPERATIUS
 * Pràctica
 * ================================
 * Autors:
 *  - Oriol Ramis (oriol.ramis)
 *  - Lluís Masdeu (lluis.masdeu)
 * ================================
 * Ports assignats:
 *  - Lluís Masdeu (8270 - 8279)
 *  - Oriol Ramis (8330 - 8339)
 */
#ifndef _LIONEL_CONSTELLATIONS_LIST_H
#define _LIONEL_CONSTELLATIONS_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERROR_NODE "Error! The node couldn't be created.\n"

typedef struct _constellationnode {
  char * code;
  double density;
  double magnitude;
  struct _constellationnode * next;
} ConstellationNode;

typedef struct {
  ConstellationNode * first;
  ConstellationNode * prev;
} ConstellationsList;

void CONSTELLATIONS_LIST_create(ConstellationsList * l);
void CONSTELLATIONS_LIST_add(ConstellationsList * l, char * code, double density, double magnitude);
void CONSTELLATIONS_LIST_remove(ConstellationsList * l);
void CONSTELLATIONS_LIST_view(ConstellationsList l, char * code, double * density, double * magnitude);
int CONSTELLATIONS_LIST_empty(ConstellationsList l);
void CONSTELLATIONS_LIST_forward(ConstellationsList * l);
int CONSTELLATIONS_LIST_end(ConstellationsList l);
void CONSTELLATIONS_LIST_beginning(ConstellationsList * l);
void CONSTELLATIONS_LIST_destroy(ConstellationsList * l);

#endif
