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
#include "McGruder_Configuration.h"
#include "McGruder_Connection.h"
#include "McGruder_List.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>

#define REQUIRED_ARGUMENTS 2
#define ARGUMENTS_ERROR "Error! There aren't enough execution arguments.\n"

int isRunning = 1;

/**
 * Funció encarregada d'interrompre la connexió.
 */
void interruptConnection() {
  isRunning = 0;
}

/**
 * Funció principal del programa.
 * @param  argc Número de paràmetres d'execució.
 * @param  argv Paràmetres d'execució.
 * @return      Final d'execució.
 */
int main(int argc, char ** argv) {
  int socketFd;
  char *fileContent;
  ConfigParams *configParams = (ConfigParams*) malloc(sizeof(ConfigParams));

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
  CONFIGURATION_parseFileContent(fileContent, configParams);

  // Alliberem memòria del contingut del fitxer.
  free(fileContent);
  fileContent = NULL;

  // Provem de connectar-nos amb el servidor.
  socketFd = CONNECTION_connectToServer(configParams->ipAddress, configParams->port);

  // Comprovem que la connexió s'ha establert correctament.
  if (socketFd == -1) {
    // Alliberem memòria dels paràmetres del fitxer.
    free(configParams);
    configParams = NULL;

    return -1;
  }

  // Controlem la interrupció del programa.
  signal(SIGINT, interruptConnection);

  // Executem el bucle principal.
  CONNECTION_programExecution(socketFd, configParams);

  // Alliberem memòria dels paràmetres del fitxer.
  free(configParams);
  configParams = NULL;

  // Tanquem el socket de la comunicació amb el client.
  close(socketFd);

  return 0;
}
