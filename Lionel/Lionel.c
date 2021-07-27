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
#include <pthread.h>
#include "Lionel_Configuration.h"
#include "Lionel_Connection.h"
#include "Lionel_List.h"
#include "Lionel_Constellations_List.h"
#include "Paquita.h"

#define REQUIRED_ARGUMENTS 2
#define ARGUMENTS_ERROR "Error! There aren't enough execution arguments.\n"
#define MSG_SERVER_INIT "Starting Lionel.\n"

int serverFd, idPaquita, serverRunning = 1;
List imagesList, textFilesList;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
Clients clients;

/**
 * Funció encarregada d'interrompre la connexió.
 */
void interruptConnection() {
  serverRunning = 0;
}

/**
 * Funció encarregada d'inicialitzar la cua de missatges.
 * @param  keyPaquita Clau de la cua de missatges de Paquita.
 * @return            0 si no hi ha hagut cap problema. -1 si hi ha hagut problemes.
 */
int initializeMessagesQueue(key_t * keyPaquita) {
  *keyPaquita = ftok("Paquita.c", 0);

  // Comprovem si hi ha hagut algun problema en la inicialització.
  if (*keyPaquita == (key_t) -1) {
    return -1;
  }

  // Obtenim l'identificador de la cua de missatges.
  idPaquita = msgget(*keyPaquita, 0600 | IPC_CREAT);

  // Comprovem si ha sigut satisfactori.
  if (idPaquita < 0) {
    msgctl(idPaquita, IPC_RMID, (struct msqid_ds *) NULL);

    return -1;
  }

  return 0;
}

/**
 * Funció encarregada de destruïr la cua de missatges.
 */
void destroyMessagesQueue() {
  msgctl(idPaquita, IPC_RMID, (struct msqid_ds *) NULL);
}

/**
 * Funció encarregada de gestionar els threads principals del programa.
 * @param isRunning     Flag que marca el funcionament de Lionel.
 * @param tLionelServer Identificador del thread.
 */
void manageThreads(int * isRunning, pthread_t * tLionelServer) {
  // Creem el fil d'execució del servidor Lionel.
  pthread_create(tLionelServer, NULL, CONNECTION_runServer, isRunning);
}

/**
 * Funció encarregada de crear les llistes dels fitxers.
 */
void createFileLists() {
  LIST_create(&imagesList);
  LIST_create(&textFilesList);
}

/**
 * Funció encarregada de destruir les llistes dels fitxers.
 */
void destroyFileLists() {
  LIST_destroy(&imagesList);
  LIST_destroy(&textFilesList);
}

/**
 * Funció encarregada de notificar a Paquita que ha mort.
 */
void notifyDeathOfPaquita() {
  Message * message = (Message*) malloc(sizeof(Message));

  message->queueType = 0;
  message->killPaquita = 1;

  // Enviem la dada.
  msgsnd(idPaquita, message, sizeof(Message), IPC_NOWAIT);

  // Alliberem la memòria demanada.
  free(message);
  message = NULL;
}

/**
 * Funció encarregada d'executar Lionel.
 * @param isRunning Flag que marca el funcionament de Lionel.
 */
void executeLionel(int * isRunning) {
  pthread_t tLionelServer;

  // Controlem els threads del programa.
  manageThreads(isRunning, &tLionelServer);
  pthread_join(tLionelServer, NULL);

  // Notifiquem a Paquita que ha mort :'(.
  notifyDeathOfPaquita();

  // Bloqueig del fil principal.
  while (*isRunning == 1) pause();

  // Tanquem el socket del servidor.
  close(serverFd);

  // Destruïm les llistes del fitxer, i el semàfor encarregat de controlar-les.
  destroyFileLists();

  // Destruïm la cua de missatges.
  destroyMessagesQueue();

  // Notifiquem la desconnexió de Lionel.
  write(1, DISCONNECTING_LIONEL, strlen(DISCONNECTING_LIONEL));
}

/**
 * Funció encarregada de crear el fork encarregat d'executar Paquita.
 * @param  isRunning Flag que marca el funcionament de Lionel.
 * @return           0
 */
int executePaquita(int * isRunning) {
  int pid = fork();

  if (pid > 0) {    // Pare
    executeLionel(isRunning);
    wait(&pid);
  } else {          // Fill
    PAQUITA_mainPaquita();
  }

  return 0;
}

/**
 * Funció principal del programa.
 * @param  argc Número de paràmetres d'execució.
 * @param  argv Paràmetres d'execució.
 * @return      Final d'execució.
 */
int main(int argc, char ** argv) {
  int isRunning = 1;
  char *fileContent;
  key_t keyPaquita;
  ConfigParams configParams;

  // Comprovem que s'han introduït els paràmetres d'execució necessaris.
  if (argc != REQUIRED_ARGUMENTS) {
    write(1, ARGUMENTS_ERROR, strlen(ARGUMENTS_ERROR));
    return -1;
  }

  // Intentem obrir el fitxer, i desar-ne el contingut.
  if (CONFIGURATION_openFile(argv[1], &fileContent) == -1) {
    return -1;
  }

  // Llegim el contingut del fitxer.
  CONFIGURATION_parseFileContent(fileContent, &configParams);

  // Alliberem memòria del contingut del fitxer.
  free(fileContent);
  fileContent = NULL;

  // Notifiquem sobre la inicialització del servidor.
  write(1, MSG_SERVER_INIT, sizeof(MSG_SERVER_INIT));

  // Creem les cues de missatges.
  if (initializeMessagesQueue(&keyPaquita) == -1) {
    return -1;
  }

  // Intentem crear la comunicació del servidor.
  serverFd = CONNECTION_connectServer(configParams.ipAddress, configParams.portMcGruder);

  // Comprovem que s'ha dut a terme correctament.
  if (serverFd == -1) {
    return -1;
  }

  // Creem les llistes dels fitxers, i el semàfor encarregat de controlar-les.
  createFileLists();

  // Controlem el CTRL+C.
  signal(SIGINT, interruptConnection);

  // Executem Paquita.
  executePaquita(&isRunning);

  return 0;
}
