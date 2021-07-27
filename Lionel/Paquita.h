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
#ifndef _PAQUITA_H
#define _PAQUITA_H
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <stdint.h>

#define ERROR_CREATING_QUEUES "Error! The messages queue couldn't been initialized.\n"
#define IMAGE_FILE_TYPE_QUEUE 1
#define TEXT_FILE_TYPE_QUEUE 2
#define FILES_TYPE_QUEUE 101
#define CONSTELLATIONS_TYPE_QUEUE 202
#define FEEDBACK_TYPE_QUEUE 303

#define CODE_MAX 10

typedef struct {
  int fileType;                           // Tipus de missatge (IMAGE_FILE_TYPE_QUEUE / TEXT_FILE_TYPE_QUEUE) (fitxer).
  long fileSize;                          // Tamany del fitxer (en Bytes) (fitxer).
} FilesMessagePaquita;

typedef struct {
  char code[CODE_MAX];                    // Codi de la constel·lació (constel·lació).
  float density;                          // Densitat de la constel·lació (constel·lació).
  float magnitude;                        // Magnitud de la constel·lació (constel·lació).
} ConstellationsMessagePaquita;

typedef struct {
  int imageFiles;                         // Número d'imatges rebudes.
  int textFiles;                          // Número de fitxers de text rebuts.
  double totalSize;                       // Tamany total de tots els fitxers (en KB).
  float averageConstellations;            // Mitjana de les constel·lacions pel nombre de fitxers.
  int constellationsNumber;               // Número total de constel·lacions.
  float averageDensity;                   // Mitjana de les densitats de les constel·lacions.
  float maxMagnitude;                     // Magnitud màxima de les constel·lacions.
  float minMagnitude;                     // Magnitud mínima de les constel·lacions.
} StatisticsPaquita;

typedef struct {
  int queueType;                          // Identificador del tipus de cua.
  int killPaquita;                        // Senyal per matar a Paquita.
  FilesMessagePaquita fMessage;           // Informació dels fitxers.
  ConstellationsMessagePaquita cMessage;  // Informació de les constel·lacions.
  StatisticsPaquita statistics;           // Resultat
} MessagePaquita;

void PAQUITA_mainPaquita();

#endif
