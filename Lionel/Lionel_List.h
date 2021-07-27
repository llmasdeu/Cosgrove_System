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
#ifndef _LIONEL_LIST_H
#define _LIONEL_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERROR_NODE "Error! The node couldn't be created.\n"

typedef struct _node {
  char * date;
  long fileSize;
  struct _node * next;
} Node;

typedef struct {
  Node * first;
  Node * prev;
} List;

void LIST_create(List * l);
void LIST_add(List * l, char * date, long fileSize);
void LIST_remove(List * l);
void LIST_view(List l, char ** date, long * fileSize);
int LIST_empty(List l);
void LIST_forward(List * l);
int LIST_end(List l);
void LIST_beginning(List * l);
void LIST_destroy(List * l);

#endif
