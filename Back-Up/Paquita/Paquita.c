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
#include "Paquita_List.h"

#define ERROR_CREATING_QUEUES "Error! The message queues couldn't been initialized.\n"
#define IMAGE_FILE_TYPE_QUEUE 1
#define TEXT_FILE_TYPE_QUEUE 2

#define CODE_MAX 5

typedef struct {
  int fileType;                 // Tipus de missatge (IMAGE_FILE_TYPE_QUEUE / TEXT_FILE_TYPE_QUEUE).
  long fileSize;                // Tamany del fitxer (en Bytes).
} FileMessage;

typedef struct {
  char code[CODE_MAX];          // Codi de la constel·lació.
  float density;                // Densitat de la constel·lació.
  float magnitude;              // Magnitud de la constel·lació.
} ConstellationMessage;

typedef struct {
  int imageFiles;
  int textFiles;
  double totalSize;
  float averageConstellations;
  int constellationsNumber;
  float averageDensity;
  float maxMagnitude;
  float minMagnitude;
} FeedbackMessage;

typedef struct {
  int imageFiles;               // Número d'imatges rebudes.
  int textFiles;                // Número de fitxers de text rebuts.
  double totalSize;             // Tamany total de tots els fitxers (en KB).
  float averageConstellations;  // Mitjana de les constel·lacions pel nombre de fitxers.
  int constellationsNumber;     // Número total de constel·lacions.
  float averageDensity;         // Mitjana de les densitats de les constel·lacions.
  float maxMagnitude;           // Magnitud màxima de les constel·lacions.
  float minMagnitude;           // Magnitud mínima de les constel·lacions.
} Statistics;

int paquitaRunning = 1, idFile, idConstellation, idFeedback;
Statistics statistics;

/**
 * Funció encarregada d'interrompre la connexió.
 */
void interruptConnection() {
  paquitaRunning = 0;
}

/**
 * Funció encarregada d'inicialitzar les cues de missatges.
 * @param  keyFile          Clau de la cua de missatges dels fitxers.
 * @param  keyConstellation Clau de la cua de missatges de les constel·lacions.
 * @param  keyFeedback      Clau de la cua de missatges de la resposta.
 * @return                  0 si no hi ha hagut cap problema. -1 si hi ha hagut problemes.
 */
int initializeMessagesQueues(key_t * keyFile, key_t * keyConstellation, key_t * keyFeedback) {
  *keyFile = ftok("Paquita.c", 0);
  *keyConstellation = ftok("Paquita.c", 1);
  *keyFeedback = ftok("Paquita.c", 2);

  // Comprovem si hi ha hagut algun problema en la inicialització.
  if (*keyFile == (key_t) -1 || *keyConstellation == (key_t) -1 || *keyFeedback == (key_t) -1) {
    return -1;
  }

  // Obtenim els identificadors de les cues de missatges.
  idFile = msgget(*keyFile, 0600 | IPC_CREAT);
  idConstellation = msgget(*keyConstellation, 0600 | IPC_CREAT);
  idFeedback = msgget(*keyFeedback, 0600 | IPC_CREAT);

  // Comprovem si ha sigut satisfactori.
  if (idFile < 0 || idConstellation < 0 || idFeedback < 0) {
    if (idFile >= 0) msgctl(idFile, IPC_RMID, (struct msqid_ds *) NULL);
    if (idConstellation >= 0) msgctl(idConstellation, IPC_RMID, (struct msqid_ds *) NULL);
    if (idFeedback >= 0) msgctl(idFeedback, IPC_RMID, (struct msqid_ds *) NULL);

    return -1;
  }

  return 0;
}

/**
 * Funció encarregada de reiniciar les estadístiques.
 */
void resetStatistics() {
  statistics.imageFiles = 0;
  statistics.textFiles = 0;
  statistics.totalSize = 0.0;
  statistics.averageConstellations = 0.0;
  statistics.constellationsNumber = 0;
  statistics.averageDensity = 0.0;
  statistics.maxMagnitude = 0.0;
  statistics.minMagnitude = 0.0;
}

/**
 * Funció encarregada de definir la informació de la resposta.
 * @param feedMessage [description]
 */
void setFeedbackData(FeedbackMessage * feedMessage) {
  feedMessage->imageFiles = statistics.imageFiles;
  feedMessage->textFiles = statistics.textFiles;
  feedMessage->totalSize = statistics.totalSize;
  feedMessage->averageConstellations = statistics.averageConstellations;
  feedMessage->constellationsNumber = statistics.constellationsNumber;
  feedMessage->averageDensity = statistics.averageDensity;
  feedMessage->maxMagnitude = statistics.maxMagnitude;
  feedMessage->minMagnitude = statistics.minMagnitude;
}

/**
 * Funció encarregada de gestionar la cua de missatges dels fitxers.
 */
void* filesProcess() {
  int n;
  double kb;
  FileMessage fMessage;
  FeedbackMessage feedMessage;

  while (paquitaRunning == 1) {
    //n = msgrcv(idFile, &fMessage, sizeof(FileMessage), 0, 0);
    n = msgrcv(idFile, &fMessage, sizeof(FileMessage), 0, IPC_NOWAIT);

    // Comprovem que s'ha rebut alguna cosa.
    if (n > 0) {
      printf("fileType: %d - fileSize: %ld\n", fMessage.fileType, fMessage.fileSize);
      kb = (double) fMessage.fileSize / 1000;

      statistics.totalSize += kb;

      if (fMessage.fileType == IMAGE_FILE_TYPE_QUEUE) {
        statistics.imageFiles++;
      } else if (fMessage.fileType == TEXT_FILE_TYPE_QUEUE) {
        statistics.textFiles++;
      }

      // Obtenim les dades de resposta.
      setFeedbackData(&feedMessage);

      // Enviem la resposta amb les estadístiques.
      msgsnd(idFeedback, &feedMessage, sizeof(FeedbackMessage), IPC_NOWAIT);
    }
  }

  return (void*) 0;
}

/**
 * Funció encarregada de desar les dades de les constel·lacions.
 * @param cList    Llista de les constel·lacions.
 * @param cMessage Dades de la constel·lació.
 */
void storeConstellationData(List * cList, ConstellationMessage cMessage) {
  Constellation * constellation = (Constellation*) malloc(sizeof(Constellation));
  constellation->code = (char*) malloc((strlen(cMessage.code) + 1) * sizeof(char));

  // Copiem les dades de la constel·lació.
  strcpy(constellation->code, cMessage.code);
  constellation->density = cMessage.density;
  constellation->magnitude = cMessage.magnitude;

  // Busquem la constel·lació en el registre.
  if (LIST_search(cList, cMessage.code) == 0) {
    LIST_add(cList, *constellation);
  }

  // Alliberem la memòria demanada.
  free(constellation);
  constellation = NULL;
}

/**
 * Funció encarregada de computar les estadístiques de les constel·lacions.
 * @param cList       Llista de les constel·lacions.
 */
void computeConstellationData(List * cList) {
  int times = 0, totalConstellations = 0;
  float density = 0.0;
  Constellation * constellation;

  // Anem a l'inici de la llista.
  LIST_beginning(cList);

  while (!LIST_end(*cList)) {
    constellation = (Constellation*) malloc(sizeof(Constellation));

    LIST_view(*cList, constellation);
    LIST_forward(cList);

    totalConstellations++;
    times += constellation->times;
    density += constellation->density;

    // Alliberem la memòria demanada.
    free(constellation);
    constellation = NULL;
  }

  statistics.averageConstellations = totalConstellations / statistics.textFiles;
  statistics.constellationsNumber = totalConstellations;
  statistics.averageDensity = density / times;
  statistics.minMagnitude = cList->minMagnitude;
  statistics.maxMagnitude = cList->maxMagnitude;
}

/**
 * Funció encarregada de gestionar la cua de missatges de les constel·lacions.
 */
void* constellationsProcess() {
  int n;
  List constellationsList;
  FeedbackMessage feedMessage;
  ConstellationMessage cMessage;

  // Creem la llista de les constel·lacions.
  LIST_create(&constellationsList);

  while (paquitaRunning == 1) {
    n = msgrcv(idConstellation, &cMessage, sizeof(ConstellationMessage), 0, IPC_NOWAIT);

    // Comprovem que s'ha rebut alguna cosa.
    if (n > 0) {
      storeConstellationData(&constellationsList, cMessage);
      computeConstellationData(&constellationsList);

      // Obtenim les dades de resposta.
      setFeedbackData(&feedMessage);

      // Enviem la resposta amb les estadístiques.
      msgsnd(idFeedback, &feedMessage, sizeof(FeedbackMessage), IPC_NOWAIT);
    }
  }

  // Destruim la llista de les constel·lacions.
  LIST_destroy(&constellationsList);

  return (void*) 0;
}

/**
 * Funció encarregada de gestionar els threads principals del programa.
 * @param tFile          Identificador del thread dels fitxers.
 * @param tConstellation Identificador del thread de les constel·lacions.
 */
void manageThreads(pthread_t * tFile, pthread_t * tConstellation) {
  pthread_create(tFile, NULL, filesProcess, NULL);
  pthread_create(tConstellation, NULL, constellationsProcess, NULL);
}

/**
 * Funció encarregada de destruïr les cues de missatges.
 */
void destroyMessagesQueues() {
  msgctl(idFile, IPC_RMID, (struct msqid_ds *) NULL);
  msgctl(idConstellation, IPC_RMID, (struct msqid_ds *) NULL);
  msgctl(idFeedback, IPC_RMID, (struct msqid_ds *) NULL);
}

/**
 * Funció principal del programa.
 * @return  Final d'execució.
 */
int main(void) {
  key_t keyFile, keyConstellation, keyFeedback;
  pthread_t tFile, tConstellation;

  // Intentem inicialitzar les cues de missatges.
  if (initializeMessagesQueues(&keyFile, &keyConstellation, &keyFeedback) == -1) {
    write(1, ERROR_CREATING_QUEUES, strlen(ERROR_CREATING_QUEUES));

    return -1;
  }

  // Reiniciem les estadístiques.
  resetStatistics();

  // Controlem la interrupció del programa.
  signal(SIGINT, interruptConnection);

  // Controlem els threads del programa.
  manageThreads(&tFile, &tConstellation);
  pthread_join(tFile, NULL);
  pthread_join(tConstellation, NULL);

  // Bloqueig del fil principal.
  while (paquitaRunning == 1) pause();

  // Destruïm les cues de missatges.
  destroyMessagesQueues();

  return 0;
}
