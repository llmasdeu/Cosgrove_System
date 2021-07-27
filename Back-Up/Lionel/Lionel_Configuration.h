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
#ifndef _LIONEL_CONFIGURATION_H
#define _LIONEL_CONFIGURATION_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>

#define DAT_FILE_ERROR "Error! The .dat file couldn't be opened.\n"

typedef struct _configparams {
  char * ipAddress;
  int portMcGruder;
  int portMcTavish;
  int time;
} ConfigParams;

int CONFIGURATION_openFile(char * fileName, char ** fileContent);
void CONFIGURATION_parseFileContent(char * fileContent, ConfigParams * configParams);

#endif
