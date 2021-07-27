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
#include "Lionel_List.h"

/**
 * Funció encarregada de crear una llista.
 * @param l Llista.
 */
void LIST_create(List * l) {
  l->first = (Node*) malloc(sizeof(Node));

	if (l->first == NULL) {
    write(1, ERROR_NODE, strlen(ERROR_NODE));
	} else {
		l->prev = l->first;
		l->first->next = NULL;
	}
}

/**
 * Funció encarregada d'inserir un fitxer a la llista.
 * @param l        Llista.
 * @param date     Data de recepció.
 * @param fileSize Tamany de la imatge.
 */
void LIST_add(List * l, char * date, long fileSize) {
  Node * aux = (Node*) malloc(sizeof(Node));

	if (aux == NULL) {
		write(1, ERROR_NODE, strlen(ERROR_NODE));
	} else {
		aux->date = (char*) malloc(strlen(date) * sizeof(char));
    strcpy(aux->date, date);
    aux->fileSize = fileSize;

		aux->next = l->prev->next;
		l->prev->next = aux;
		l->prev = l->prev->next;
	}
}

/**
 * Funció encarregada d'esborrar un nom de la llista.
 * @param l Llista.
 */
void LIST_remove(List * l) {
  Node * aux;

  if (!LIST_end(*l)) {
    aux = l->prev->next;
    l->prev->next = l->prev->next->next;

    free(aux->date);
    free(aux);
  }
}

/**
 * Funció encarregada d'obtenir el nom del punt d'interès de la llista.
 * @param l        Llista.
 * @param date     Data de recepció del fitxer.
 * @param fileSize Tamany de la imatge.
 */
void LIST_view(List l, char ** date, long * fileSize) {
  if (!LIST_end(l)) {
		*date = (char*) malloc(strlen(l.prev->next->date) * sizeof(char));
    strcpy(*date, l.prev->next->date);
    *fileSize = l.prev->next->fileSize;
	}
}

/**
 * Funció encarregada de saber si la llista es troba buida.
 * @param  l Llista.
 * @return   CERT si es troba buida. FALS si té algun nom desat.
 */
int  LIST_empty(List l) {
  return l.first->next == NULL;
}

/**
 * Funció encarregada d'avançar el punt d'interès de la llista.
 * @param l Llista.
 */
void LIST_forward(List * l) {
  if (!LIST_end(*l)) {
    l->prev = l->prev->next;
  }
}

/**
 * Funció encarregada de saber si el punt d'interès de la llista es troba al final.
 * @param  l Llista.
 * @return   CERT si es troba al final. FALS si no es troba al final.
 */
int  LIST_end(List l) {
  return l.prev->next == NULL;
}

/**
 * Funció encarregada de moure el punt d'interés de la llista a l'inici.
 * @param l Llista.
 */
void LIST_beginning(List * l) {
  l->prev = l->first;
}

/**
 * Funció encarregada de destruir una llista.
 * @param l Llista.
 */
void LIST_destroy(List * l) {
  LIST_beginning(l);

	while (!LIST_end(*l)) {
		LIST_remove(l);
	}

	free(l->first);
	l->first = NULL;
	l->prev = NULL;
}
