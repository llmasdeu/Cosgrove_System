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
#include "Paquita_List.h"

/**
 * Funció encarregada de crear una llista.
 * @param l Llista.
 */
void PAQUITA_LIST_create(List * l) {
  l->first = (Node*) malloc(sizeof(Node));

	if (l->first == NULL) {
    write(1, ERROR_NODE, strlen(ERROR_NODE));
	} else {
		l->prev = l->first;
		l->first->next = NULL;

    l->maxMagnitude = -5000;
    l->minMagnitude = 5000;
	}
}

/**
 * Funció encarregada d'inserir una constel·lació a la llista.
 * @param l             Llista.
 * @param constellation Constel·lació a inserir.
 */
void PAQUITA_LIST_add(List * l, Constellation constellation) {
  Node * aux = (Node*) malloc(sizeof(Node));

	if (aux == NULL) {
		write(1, ERROR_NODE, strlen(ERROR_NODE));
	} else {
		aux->code = (char*) malloc(strlen(constellation.code) * sizeof(char));
    strcpy(aux->code, constellation.code);
    aux->density = constellation.density;
    aux->magnitude = constellation.magnitude;
    aux->times = 1;

		aux->next = l->prev->next;
		l->prev->next = aux;
		l->prev = l->prev->next;

    if (constellation.magnitude < l->minMagnitude) {
      l->minMagnitude = constellation.magnitude;
    }

    if (constellation.magnitude > l->maxMagnitude) {
      l->maxMagnitude = constellation.magnitude;
    }
	}
}

/**
 * Funció encarregada d'esborrar una constel·lació de la llista.
 * @param l Llista.
 */
void PAQUITA_LIST_remove(List * l) {
  Node * aux;

  if (!PAQUITA_LIST_end(*l)) {
    aux = l->prev->next;
    l->prev->next = l->prev->next->next;
    free(aux);
  }
}

/**
 * Funció encarregada d'obtenir la constel·lació del punt d'interès de la llista.
 * @param l             Llista.
 * @param constellation Constel·lació a consultar.
 */
void PAQUITA_LIST_view(List l, Constellation * constellation) {
  if (!PAQUITA_LIST_end(l)) {
		constellation->code = (char*) malloc(strlen(l.prev->next->code) * sizeof(char));
    strcpy(constellation->code, l.prev->next->code);
    constellation->density = l.prev->next->density;
    constellation->magnitude = l.prev->next->magnitude;
    constellation->times = l.prev->next->times;
	}
}

/**
 * Funció encarregada de cercar una constel·lació a la llista.
 * @param  l                 Llista.
 * @param  constellationName Nom de la constel·lació a cercar.
 * @return                   1 si es troba a la llista. 0 si no es troba.
 */
int PAQUITA_LIST_search(List * l, char * constellationName) {
  int found = 0;
  Constellation aux;

  PAQUITA_LIST_beginning(l);

  while (!PAQUITA_LIST_end(*l) && !found) {
    PAQUITA_LIST_view(*l, &aux);
    PAQUITA_LIST_forward(l);

    if (strcmp(aux.code, constellationName) == 0) {
      found = 1;
    }
  }

  return found;
}

/**
 * Funció encarregada d'actualitzar les dades de la constel·lació.
 * @param  l             Llista.
 * @param  constellation Dades de la constel·lació a actualitzar.
 * @return               1 si no hi ha hagut cap problema. 0 si hi ha hagut problemes.
 */
int PAQUITA_LIST_update(List * l, Constellation constellation) {
  if (PAQUITA_LIST_search(l, constellation.code) == 0) {
    l->prev->next->density += constellation.density;
    l->prev->next->magnitude += constellation.magnitude;
    l->prev->next->times++;

    if (constellation.magnitude < l->minMagnitude) {
      l->minMagnitude = constellation.magnitude;
    }

    if (constellation.magnitude > l->maxMagnitude) {
      l->maxMagnitude = constellation.magnitude;
    }

    return 1;
  }

  return 0;
}

/**
 * Funció encarregada de saber si la llista es troba buida.
 * @param  l Llista.
 * @return   CERT si es troba buida. FALS si té algun nom desat.
 */
int PAQUITA_LIST_empty(List l) {
  return l.first->next == NULL;
}

/**
 * Funció encarregada d'avançar el punt d'interès de la llista.
 * @param l Llista.
 */
void PAQUITA_LIST_forward(List * l) {
  if (!PAQUITA_LIST_end(*l)) {
    l->prev = l->prev->next;
  }
}

/**
 * Funció encarregada de saber si el punt d'interès de la llista es troba al final.
 * @param  l Llista.
 * @return   CERT si es troba al final. FALS si no es troba al final.
 */
int PAQUITA_LIST_end(List l) {
  return l.prev->next == NULL;
}

/**
 * Funció encarregada de moure el punt d'interés de la llista a l'inici.
 * @param l Llista.
 */
void PAQUITA_LIST_beginning(List * l) {
  l->prev = l->first;
}

/**
 * Funció encarregada de destruir una llista.
 * @param l Llista.
 */
void PAQUITA_LIST_destroy(List * l) {
  PAQUITA_LIST_beginning(l);

	while (!PAQUITA_LIST_end(*l)) {
		PAQUITA_LIST_remove(l);
	}

	free(l->first);
	l->first = NULL;
	l->prev = NULL;
}
