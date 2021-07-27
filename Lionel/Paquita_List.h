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
#ifndef _PAQUITA_LIST_H
#define _PAQUITA_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERROR_NODE "Error! The node couldn't be created.\n"

typedef struct _node {
  char * code;            // Codi de la constel·lació.
  float density;          // Densitat de la constel·lació.
  float magnitude;        // Magnitud de la constel·lació.
  int times;              // Número de vegades que s'ha rebut dades de la constel·lació.
  struct _node * next;
} Node;

typedef struct {
  Node * first;
  Node * prev;
  float maxMagnitude;
  float minMagnitude;
} List;

typedef struct {
  char * code;
  float density;
  float magnitude;
  int times;
} Constellation;

void PAQUITA_LIST_create(List * l);
void PAQUITA_LIST_add(List * l, Constellation constellation);
void PAQUITA_LIST_remove(List * l);
void PAQUITA_LIST_view(List l, Constellation * constellation);
int PAQUITA_LIST_search(List * l, char * constellationName);
int PAQUITA_LIST_update(List * l, Constellation constellation);
int PAQUITA_LIST_empty(List l);
void PAQUITA_LIST_forward(List * l);
int PAQUITA_LIST_end(List l);
void PAQUITA_LIST_beginning(List * l);
void PAQUITA_LIST_destroy(List * l);

#endif
