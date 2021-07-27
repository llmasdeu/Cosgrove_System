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
#include "Lionel_Configuration.h"

/**
 * Funció encarrega d'obrir un fitxer, i desar-ne el contingut.
 * @param  fileName    Nom del fitxer.
 * @param  fileContent Contingut del fitxer (referència).
 * @return             Si s'ha obert correctament: 0.
 *                     Si no s'ha obert correctament: -1.
 */
int CONFIGURATION_openFile(char * fileName, char ** fileContent) {
  int fd = open(fileName, O_RDONLY), fileSize;

  if (fd < 0) {
    write(1, DAT_FILE_ERROR, strlen(DAT_FILE_ERROR));

    return -1;
  } else {
    fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    //hola
    *fileContent = malloc(fileSize * sizeof(char));
    read(fd, *fileContent, fileSize);

    close(fd);
  }

  return 0;
}

/**
 * Funció encarregada de separar el contingut del fitxer pel caràcter '\n'.
 * @param fileContent Contingut del fitxer.
 * @param fileParam   Paràmetre a desar (per referència).
 * @param i           Índex del contingut de fitxer actual (per referència).
 */
void CONFIGURATION_splitFileContent(char * fileContent, char ** fileParam, int * i) {
  int j = 0, currentSize = 1;

  *fileParam = malloc(sizeof(char));

  while (fileContent[*i] != '\n') {
    *fileParam = realloc(*fileParam, currentSize + 1);
    (*fileParam)[j] = fileContent[*i];
    currentSize++;
    j++;
    (*i)++;
  }

  (*fileParam)[j] = '\0';
}

/**
 * Funció encarregada de parsejar el contingut dels fitxers.
 * @param fileContent  Contingut del fitxer.
 * @param configParams Paràmetres de configuració.
 */
void CONFIGURATION_parseFileContent(char * fileContent, ConfigParams * configParams) {
  int i = 0;
  char * fileParam;

  // Adreça IP
  CONFIGURATION_splitFileContent(fileContent, &fileParam, &i);
  configParams->ipAddress = malloc(strlen(fileParam) * sizeof(char));
  strcpy(configParams->ipAddress, fileParam);
  free(fileParam);
  fileParam = NULL;
  i++;

  // Port McGruder
  CONFIGURATION_splitFileContent(fileContent, &fileParam, &i);
  configParams->portMcGruder = atoi(fileParam);
  free(fileParam);
  fileParam = NULL;
  i++;

  // Port McTavish
  CONFIGURATION_splitFileContent(fileContent, &fileParam, &i);
  configParams->portMcTavish = atoi(fileParam);
  free(fileParam);
  fileParam = NULL;
  i++;

  // Temps
  CONFIGURATION_splitFileContent(fileContent, &fileParam, &i);
  configParams->time = atoi(fileParam);
  free(fileParam);
  fileParam = NULL;
}
