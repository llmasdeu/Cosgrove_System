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
#include "Lionel_Constellations_List.h"

/**
 * Funció encarregada de crear una llista.
 * @param l Llista.
 */
void CONSTELLATIONS_LIST_create(ConstellationsList * l) {
  l->first = (ConstellationNode*) malloc(sizeof(ConstellationNode));

	if (l->first == NULL) {
    write(1, ERROR_NODE, strlen(ERROR_NODE));
	} else {
		l->prev = l->first;
		l->first->next = NULL;
	}
}

/**
 * Funció encarregada d'inserir una constel·lació a la llista.
 * @param l         Llista.
 * @param code      Codi de la constel·lació.
 * @param density   Densitat de la constel·lació.
 * @param magnitude Magnitud de la constel·lació.
 */
void CONSTELLATIONS_LIST_add(ConstellationsList * l, char * code, double density, double magnitude) {
  ConstellationNode * aux = (ConstellationNode*) malloc(sizeof(ConstellationNode));

	if (aux == NULL) {
		write(1, ERROR_NODE, strlen(ERROR_NODE));
	} else {
		aux->code = (char*) malloc(strlen(code) * sizeof(char));
    strcpy(aux->code, code);
    aux->density = density;
    aux->magnitude = magnitude;

		aux->next = l->prev->next;
		l->prev->next = aux;
		l->prev = l->prev->next;
	}
}

/**
 * Funció encarregada d'esborrar una constel·lació de la llista.
 * @param l Llista.
 */
void CONSTELLATIONS_LIST_remove(ConstellationsList * l) {
  ConstellationNode * aux;

  if (!CONSTELLATIONS_LIST_end(*l)) {
    aux = l->prev->next;
    l->prev->next = l->prev->next->next;

    free(aux->code);
    free(aux);
  }
}

/**
 * Funció encarregada d'obtenir les dades d'una constel·lació de la llista.
 * @param l         Llista.
 * @param code      Codi de la constel·lació.
 * @param density   Densitat de la constel·lació.
 * @param magnitude Magnitud de la constel·lació.
 */
void CONSTELLATIONS_LIST_view(ConstellationsList l, char * code, double * density, double * magnitude) {
  if (!CONSTELLATIONS_LIST_end(l)) {
    strcpy(code, l.prev->next->code);
    *density = l.prev->next->density;
    *magnitude = l.prev->next->magnitude;
	}
}

/**
 * Funció encarregada de saber si la llista es troba buida.
 * @param  l Llista.
 * @return   CERT si es troba buida. FALS si té algun nom desat.
 */
int  CONSTELLATIONS_LIST_empty(ConstellationsList l) {
  return l.first->next == NULL;
}

/**
 * Funció encarregada d'avançar el punt d'interès de la llista.
 * @param l Llista.
 */
void CONSTELLATIONS_LIST_forward(ConstellationsList * l) {
  if (!CONSTELLATIONS_LIST_end(*l)) {
    l->prev = l->prev->next;
  }
}

/**
 * Funció encarregada de saber si el punt d'interès de la llista es troba al final.
 * @param  l Llista.
 * @return   CERT si es troba al final. FALS si no es troba al final.
 */
int  CONSTELLATIONS_LIST_end(ConstellationsList l) {
  return l.prev->next == NULL;
}

/**
 * Funció encarregada de moure el punt d'interés de la llista a l'inici.
 * @param l Llista.
 */
void CONSTELLATIONS_LIST_beginning(ConstellationsList * l) {
  l->prev = l->first;
}

/**
 * Funció encarregada de destruir una llista.
 * @param l Llista.
 */
void CONSTELLATIONS_LIST_destroy(ConstellationsList * l) {
  CONSTELLATIONS_LIST_beginning(l);

	while (!CONSTELLATIONS_LIST_end(*l)) {
		CONSTELLATIONS_LIST_remove(l);
	}

	free(l->first);
	l->first = NULL;
	l->prev = NULL;
}
