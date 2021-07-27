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

#define REQUIRED_ARGUMENTS 2
#define ARGUMENTS_ERROR "Error! There aren't enough execution arguments.\n"
#define MSG_SERVER_INIT "Starting Lionel.\n"

int serverFd, idFile, idConstellation, idFeedback, serverRunning = 1;
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
 * Funció encarregada d'inicialitzar les cues de missatges.
 * @param  keyFile          Clau de la cua de missatges dels fitxers.
 * @param  keyConstellation Clau de la cua de missatges de les constel·lacions.
 * @param  keyFeedback      Clau de la cua de missatges de la resposta.
 * @return                  0 si no hi ha hagut cap problema. -1 si hi ha hagut problemes.
 */
int initializeMessagesQueues(key_t * keyFile, key_t * keyConstellation, key_t * keyFeedback) {
   *keyFile = ftok(PAQUITA_PATH, 0);
   *keyConstellation = ftok(PAQUITA_PATH, 1);
   *keyFeedback = ftok(PAQUITA_PATH, 2);

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
 * Funció encarregada de destruïr les cues de missatges.
 */
void destroyMessagesQueues() {
  msgctl(idFile, IPC_RMID, (struct msqid_ds *) NULL);
  msgctl(idConstellation, IPC_RMID, (struct msqid_ds *) NULL);
  msgctl(idFeedback, IPC_RMID, (struct msqid_ds *) NULL);
}

/**
 * Funció encarregada de gestionar els threads principals del programa.
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
 * Funció principal del programa.
 * @param  argc Número de paràmetres d'execució.
 * @param  argv Paràmetres d'execució.
 * @return      Final d'execució.
 */
int main(int argc, char ** argv) {
  int isRunning = 1;
  char *fileContent;
  key_t keyFile, keyConstellation, keyFeedback;
  pthread_t tLionelServer;
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
  if (initializeMessagesQueues(&keyFile, &keyConstellation, &keyFeedback) == -1) {
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

  // Controlem els threads del programa.
  manageThreads(&isRunning, &tLionelServer);
  pthread_join(tLionelServer, NULL);

  // Bloqueig del fil principal.
  while (isRunning == 1) pause();

  // Tanquem el socket del servidor.
  close(serverFd);

  // Destruïm les llistes del fitxer, i el semàfor encarregat de controlar-les.
  destroyFileLists();

  // Destruïm les cues de missatges.
  destroyMessagesQueues();

  return 0;
}
