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
#include "McGruder_Connection.h"

extern int isRunning;

/**
 * Funció encarregada de connectar-se a un servidor.
 * @param  address Adreça del servidor.
 * @param  port    Port del servidor.
 * @return         File Descriptor del socket.
 *                 En cas que hi hagi error, tornem un -1.
 */
int CONNECTION_connectToServer(char * ipAddress, int port) {
  struct sockaddr_in sAddrIn;
  int c, socketFd;

  // Intentem crear el socket.
  socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (socketFd < 0) {
    write(1, SOCKET_ERROR, strlen(SOCKET_ERROR));

    return -1;
  }

  // Definim els paràmetres de la connexió.
  memset(&sAddrIn, 0, sizeof(sAddrIn));

  sAddrIn.sin_family = AF_INET;
  sAddrIn.sin_port = htons(port);
  sAddrIn.sin_addr.s_addr = inet_addr(ipAddress);

  // Intentem connectar el socket.
  c = connect(socketFd, (struct sockaddr *) &sAddrIn, sizeof(sAddrIn));

  if (c < 0) {
    write(1, CONNECTION_ERROR, strlen(CONNECTION_ERROR));
    close(socketFd);

    return -1;
  }

  return socketFd;
}

/**
 * Funció encarregada d'escanejar la carpeta.
 * @param filesDirectory  Directori dels fitxers.
 * @param readFiles       Fitxers llegits.
 * @param newFiles        Fitxers a llegir.
 */
void CONNECTION_scan(char * filesDirectory, List * readFiles, List * newFiles) {
  //int tYear, tMonth, tDay, tHour, tMinutes, tSeconds;
  DIR * dr = opendir(filesDirectory);        // Retorna pointer del tipus DIR.
  /*char * creationDate = (char*) malloc(strlen(FILE_CREATION_DATE) * sizeof(char));*/
  //struct stat t_stat;
  struct dirent * de;                 // Pointer per directori d'entrada.

  if (dr == NULL) {
    write(1, FILE_ERROR, sizeof(FILE_ERROR));
  } else {
    while ((de = readdir(dr)) != NULL) {
      // Llegim la data de creació del fitxer.
      //stat(de->d_name, &t_stat);
      //struct tm * timeinfo = localtime(&t_stat.st_ctime);

      // Formatem les dades de la creació del fitxer.
      /*tYear = timeinfo->tm_year + 1900;
      tMonth = timeinfo->tm_mon + 1;
      tDay = timeinfo->tm_mday;
      tHour = timeinfo->tm_hour;
      tMinutes = timeinfo->tm_min;
      tSeconds = timeinfo->tm_sec;
      sprintf(creationDate, FILE_CREATION_DATE, tYear, tMonth, tDay, tHour, tMinutes, tSeconds);*/

      // Comprovem si el fitxer ja s'havia llegit prèviament.
      if (!LIST_search(*readFiles, de->d_name)) {
        LIST_add(readFiles, de->d_name);
        LIST_add(newFiles, de->d_name);
      }
    }

    closedir(dr);
  }
}

/**
 * Funció encarregada d'obtenir la extensió del fitxer indicat.
 * @param fileName      Nom del fitxer.
 * @param fileExtension Extensió del fitxer.
 */
void CONNECTION_getFileExtension(char * fileName, char ** fileExtension) {
  unsigned int i = 0, j, size = 0;

  while (fileName[i] != '.' && i < strlen(fileName)) {
    i++;
  }

  i++;
  j = i;

  while (((fileName[i] >= 'A' && fileName[i] <= 'Z') || (fileName[i] >= 'a' && fileName[i] <= 'z')) && i < strlen(fileName)) {
    i++;
  }

  size = i - j;

  if (size > 0) {
    size++;
    *fileExtension = malloc(size * sizeof(char));

    for (i = 0; i < size; i++) {
      (*fileExtension)[i] = fileName[i + j];
    }

    (*fileExtension)[i] = '\0';
  } else {
    *fileExtension = malloc(sizeof(char));
  }
}

/**
 * Funció encarregada de comprovar el tipus de fitxer, mitjançant la seva extensió.
 * @param  fileExtension Extensió del fitxer.
 * @return               Identificador del tipus del fitxer:
 *                          - IMAGE_FILE_TYPE
 *                          - TEXT_FILE_TYPE
 *                          - Error: -1
 */
int CONNECTION_checkFileType(char * fileExtension) {
  if (strcmp(fileExtension, JPG_LOWER_FILE_FORMAT) == 0 ||
      strcmp(fileExtension, JPG_UPPER_FILE_FORMAT) == 0 ||
      strcmp(fileExtension, JPEG_LOWER_FILE_FORMAT) == 0 ||
      strcmp(fileExtension, JPEG_UPPER_FILE_FORMAT) == 0) {
        return IMAGE_FILE_TYPE;
  }

  if (strcmp(fileExtension, TXT_LOWER_FILE_FORMAT) == 0 ||
      strcmp(fileExtension, TXT_UPPER_FILE_FORMAT) == 0) {
        return TEXT_FILE_TYPE;
  }

  return -1;
}

/**
 * Funció encarregada de generar el text amb les metadades del fitxer.
 * @param fileMetadata Text a generar.
 * @param fileType     Identificador del tipus de fitxer (TEXT_FILE_TYPE o IMAGE_FILE_TYPE)-
 * @param fileSize     Mida (en bytes) del fitxer.
 * @param creationDate Nom del fitxer.
 */
void CONNECTION_buildMetadata(char ** fileMetadata, int fileType, int fileSize, char * fileName) {
  int i = 0;
  char *creationDate = (char*) malloc(MAXCREATIONDATE * sizeof(char));

  // Obtenim la data de creació del fitxer, a partir del seu nom.
  while (fileName[i] != '.') {
    // Originalment funcionava amb reallocs, però incomprensiblement en algun cas no funcionava.
    //creationDate = (char*) realloc(creationDate, size * sizeof(char));
    creationDate[i] = fileName[i];
    i++;
  }

  creationDate[i] = '\0';

  // Comprovem el tipus de fitxer.
  if (fileType == TEXT_FILE_TYPE || fileType == IMAGE_FILE_TYPE) {
    if (fileType == TEXT_FILE_TYPE) {
      asprintf(fileMetadata, FILE_METADATA, FILE_TEXT_TYPE, fileSize, creationDate);
    } else {
      asprintf(fileMetadata, FILE_METADATA, FILE_IMAGE_TYPE, fileSize, creationDate);
    }
  } else {
    *fileMetadata = (char*) malloc(sizeof(char));
    (*fileMetadata)[i] = '\0';
  }

  // Alliberem la memòria demanada.
  free(creationDate);
  creationDate = NULL;
}

/**
 * Funció encarrega d'obrir un fitxer, i desar-ne el contingut.
 * @param fd          File Descriptor del fitxer.
 * @param actualSize  Tamany a llegir del fitxer.
 * @param position    Off-set del fitxer.
 * @param fileContent Contingut del fitxer.
 */
void CONNECTION_openFile(int fd, int actualSize, int position, unsigned char ** fileContent) {
  // Comprovem que el tamany és correcte.
  if (actualSize > 0) {
    // Demanem memòria per a la cadena.
    *fileContent = (unsigned char*) malloc(actualSize * sizeof(unsigned char));

    // Llegim el fitxer.
    if (fd < 0) {
      fileContent[0] = '\0';
    } else {
      lseek(fd, position, SEEK_SET);
      read(fd, *fileContent, actualSize * sizeof(unsigned char));
    }
  } else {
    // Demanem memòria per a la cadena.
    *fileContent = (unsigned char*) malloc(sizeof(unsigned char));
    (*fileContent)[0] = '\0';
  }
}

/**
 * Funció encarregada de generar el paquet a enviar.
 * @param  operationId Tipus de operació.
 * @param  header      Capçalera del paquet.
 * @param  data        Dades del paquet.
 * @param  argLength   Tamany auxiliar de la informació a enviar
 * @return             Paquet generat.
 */
Packet CONNECTION_buildPacket(int operationId, char * header, unsigned char * data, short argLength) {
  int i;
  Packet packet;

  packet.type = operationId;
  asprintf(&packet.header, HEADER_STRUCTURE, header);

  if (argLength != 0) {
    packet.length = argLength;
  } else {
    packet.length = strlen((char*) data);
  }

  packet.data = (unsigned char*) malloc((packet.length + 1) * sizeof(unsigned char));

  for (i = 0; i < packet.length; i++) {
    packet.data[i] = data[i];
  }

  packet.data[i] = '\0';

  return packet;
}

/**
 * Funció encarregada de llegir el paquet rebut.
 * @param  clientFd File Descriptor del client.
 * @return          Paquet amb les dades rebudes.
 */
Packet CONNECTION_unbuildPacket(int clientFd) {
  int i = 0, size;
  char aux = ' ', *auxArray = (char*) malloc(MAXHEADER * sizeof(char));
  Packet packet;

  // Netegem el paquet.
  size = sizeof(packet);
  memset(&packet, 0, size);

  // Type
  read(clientFd, &packet.type, sizeof(char));
  read(clientFd, &aux, sizeof(char));           // '['

  // Header
  auxArray = (char*) malloc(sizeof(char));
  read(clientFd, &aux, sizeof(char));

  while (aux != ']') {
    // Previament implementat amb reallocs, però incomprensiblement, en alguna crida a la funció 'realloc(): invalid next size'.
    //auxArray = (char*) realloc(auxArray, size * sizeof(char));
    if (i < MAXHEADER) auxArray[i] = aux;
    read(clientFd, &aux, sizeof(char));
    i++;
  }

  read(clientFd, &aux, sizeof(char));             // '\0'
  auxArray[i] = '\0';
  packet.header = (char*) malloc(strlen(auxArray));
  strcpy(packet.header, auxArray);

  // Alliberem la memòria demanada.
  free(auxArray);
  auxArray = NULL;

  // Length
  read(clientFd, &packet.length, sizeof(short));

  //Data
  packet.data = (unsigned char*) malloc((packet.length + 1) * sizeof(unsigned char));

  if (packet.length > 0) {
    read(clientFd, packet.data, packet.length * sizeof(unsigned char));
  }

  read(clientFd, &aux, sizeof(char));       // '\0'
  packet.data[packet.length] = '\0';

  return packet;
}

/**
 * Funció encarregada de netejar el paquet.
 * @param packet Paquet a netejar.
 */
void CONNECTION_clearPacket(Packet * packet) {
  // Reiniciem el paquet.
  packet->type = ' ';
  packet->length = 0;

  // Alliberem la memòria demanada.
  free(packet->header);
  free(packet->data);
  packet->header = NULL;
  packet->data = NULL;
}

/**
 * Funció encarregada d'enviar la trama al servidor.
 * @param clientFd File Descriptor del client.
 * @param packet   Paquet a enviar.
 */
void CONNECTION_sendPacket(int clientFd, Packet * packet) {
  // Enviem el contingut del paquet.
  write(clientFd, &packet->type, sizeof(char));
  write(clientFd, packet->header, (strlen(packet->header) + 1) * sizeof(char));
  write(clientFd, &packet->length, sizeof(short));
  write(clientFd, packet->data, packet->length + 1);

  // Netegem el paquet.
  CONNECTION_clearPacket(packet);
}

/**
 * Funció encarregada d'obtenir el tamany total del fitxer.
 * @param  fd File Descriptor del fitxer.
 * @return    Tamany en bytes del fitxer.
 */
int CONNECTION_getFileSize(int fd) {
  int fileSize;

  // Obtenim el tamany total del fitxer.
  lseek(fd, 0, SEEK_SET);
  fileSize = lseek(fd, 0, SEEK_END);

  return fileSize;
}

/**
 * Funció encarregada d'esborrar un fitxer.
 * @param filePath Drecera del fitxer a esborrar.
 */
void CONNECTION_removeFile(char * filePath) {
  char * rmSystemCall;

  // Obtenim la crida del sistema.
  asprintf(&rmSystemCall, RM_SYSTEM_CALL, filePath);

  // Esborrem els fitxers no desitjats.
  system(rmSystemCall);

  // Alliberem la memòria demanada.
  free(rmSystemCall);
  rmSystemCall = NULL;
}

/**
 * Funció encarregada d'obtenir el Checksum del fitxer indicat.
 * @param  filePath Drecera del fitxer.
 * @return          Checksum MD5 del fitxer.
 */
unsigned char* CONNECTION_computeChecksum(char * filePath) {
   char *auxFilePath, *systemCall;

   // Obtenim el Checksum.
   asprintf(&systemCall, MD5SUM_SYSTEM_CALL, filePath, filePath);
   system(systemCall);

   auxFilePath = (char*) malloc(sizeof(char) * 4 + sizeof(filePath));
   strcpy(auxFilePath, filePath);
   strcat(auxFilePath, ".md5");

   int fd = open(auxFilePath, O_RDONLY);
   unsigned char aux;
   int i = 0;
   unsigned char* checksum = malloc(sizeof(unsigned char) * 33);

   // Comprovem que el fitxer s'ha obert correctament.
   if (fd > 0) {
     read(fd, &aux, sizeof(unsigned char));

     while (i < 32) {
       checksum[i] = aux;
       read(fd, &aux, sizeof(unsigned char));
       i++;
     }

     checksum[i]='\0';
   }

   // Esborrem els fitxers generats
   CONNECTION_removeFile(auxFilePath);

   // Alliberem la memòria demanada.
   free(systemCall);
   free(auxFilePath);
   systemCall = NULL;
   auxFilePath = NULL;

   return checksum;
}

/**
 * Funció encarregada de la desconnexió del programa.
 * @param  clientFd      File Descriptor del client.
 * @param  telescopeName Nom del telescopi.
 * @return               1: si hi ha hagut algun error.
 *                       0: si no hi ha hagut cap error.
 */
int CONNECTION_checkDisconnection(int clientFd, char * telescopeName) {
  // Comprovem si es troba executant.
  if (isRunning == 1) {
    return 1;
  }

  // Enviem la trama de desconnexió.
  Packet packet = CONNECTION_buildPacket(DISCONNECT_PROTOCOL, "", (unsigned char*) telescopeName, 0);
  CONNECTION_sendPacket(clientFd, &packet);

  // Rebem la resposta del servidor.
  packet = CONNECTION_unbuildPacket(clientFd);

  // Comprovem que el servidor ens deixa desconnectar.
  if (strcmp(packet.header, "CONOK") == 0) {
    CONNECTION_clearPacket(&packet);

    return 0;
  } else {
    CONNECTION_clearPacket(&packet);

    return 1;
  }
}

/**
 * Funció encarregada del protocol d'enviament d'un fitxer.
 * @param configParams Paràmetres de configuració del servidor.
 * @param socketFd     File Descriptor del client.
 * @param fileName     Nom del fitxer.
 */
void CONNECTION_sendFileProtocol(ConfigParams * configParams, int socketFd, char * fileName) {
  int fd, error = 0, fileSize, fileType, actualSize, position;
  char *message, *filePath, *fileExtension, *fileMetadata;
  Packet packet;
  unsigned char *md5Sum, *fileContent;

  // Notifiquem sobre el fitxer.
  asprintf(&message, FILE_NAME, fileName);
  write(1, message, strlen(message));

  // Alliberem la memòria demanada.
  free(message);
  message = NULL;

  // Obtenim la drecera del fitxer.
  asprintf(&filePath, FILES_DIRECTORY, configParams->filesDirectory, fileName);

  // Obtenim la extensió del fitxer.
  CONNECTION_getFileExtension(fileName, &fileExtension);

  // Obrim el fitxer.
  fd = open(filePath, O_RDONLY);

  // Comprovem que el fitxer s'ha obert correctament.
  if (fd > 0) {
    // Obtenim el tamany total del fitxer.
    fileSize = CONNECTION_getFileSize(fd);

    // Obtenim el tipus de fitxer.
    fileType = CONNECTION_checkFileType(fileExtension);

    // Construim i enviem el missatge amb les metadades del fitxer.
    CONNECTION_buildMetadata(&fileMetadata, fileType, fileSize, fileName);
    packet = CONNECTION_buildPacket(SEND_FILE_PROTOCOL, "METADATA", (unsigned char*) fileMetadata, 0);
    CONNECTION_sendPacket(socketFd, &packet);

    // Esperem la resposta.
    packet = CONNECTION_unbuildPacket(socketFd);

    // Comprovem que el feedback sigui positiu.
    if (strcmp(packet.header, "FILEOK") == 0) {
      // Netegem el paquet.
      CONNECTION_clearPacket(&packet);

      // Notifiquem sobre l'enviament del fitxer.
      asprintf(&message, SENDING, fileName);
      write(1, message, strlen(message));

      // Alliberem la memòria demanada.
      free(message);
      message = NULL;

      // Iniciem les variables necessaries per a la lectura del fitxer.
      actualSize = 0;
      position = 0;

      // Mentre estiguem llegint el fitxer, i no hi hagi cap error d'enviament.
      while (position < fileSize && error == 0) {
        // Calculem la porció a enviar.
        if (position + MAX_FRAGMENT > fileSize) {
          actualSize = fileSize - position;
        } else {
          actualSize = MAX_FRAGMENT;
        }

        // Obtenim la porció del fitxer, i l'enviem.
        CONNECTION_openFile(fd, actualSize, position, &fileContent);
        packet = CONNECTION_buildPacket(SEND_FILE_PROTOCOL, "", fileContent, actualSize);
        CONNECTION_sendPacket(socketFd, &packet);

        // Llegim la resposta del servidor.
        packet = CONNECTION_unbuildPacket(socketFd);

        // Comprovem que la resposta és satisfactòria.
        if (strcmp(packet.header, "FILEKO") == 0) {
          error = 1;
        }

        // Netegem el paquet.
        CONNECTION_clearPacket(&packet);

        // Actualitzem l'offset actual del fitxer.
        position +=  actualSize;

        // Alliberem la memòria demanada.
        free(fileContent);
        fileContent = NULL;
      }

      // Tanquem el fitxer.
      close(fd);

      if (error == 0) {
        // Enviem el final del fitxer.
        md5Sum = CONNECTION_computeChecksum(filePath);
        packet = CONNECTION_buildPacket(SEND_FILE_PROTOCOL, "ENDFILE", md5Sum, 0);
        CONNECTION_sendPacket(socketFd, &packet);

        // Llegim la resposta.
        packet = CONNECTION_unbuildPacket(socketFd);

        // Mirem que l'enviament ha sigut satisfactori.
        if (strcmp(packet.header, "CHECKOK") == 0) {
          // Notifiquem sobre el final de l'enviament del fitxer.
          write(1, FILE_SENT, strlen(FILE_SENT));

          // TODO: Esborrar el fitxer de la carpeta.
        } else {
          // Notifiquem sobre el final de l'enviament del fitxer incorrecte.
          write(1, FILE_SENT_ERROR, strlen(FILE_SENT_ERROR));
        }

        // Netegem el paquet.
        CONNECTION_clearPacket(&packet);
      }

      // Alliberem la memòria demanada.
      free(fileMetadata);
      fileMetadata = NULL;
    } else {
      write(1, SENDING_ERROR, strlen(SENDING_ERROR));

      // Netegem el paquet.
      CONNECTION_clearPacket(&packet);
    }
  } else {
    // Notifiquem de que no hem pogut obrir el fitxer.
    write(1, ERROR_OPENING_FILE, strlen(ERROR_OPENING_FILE));
  }

  // Alliberem la memòria demanada.
  free(fileExtension);
  free(filePath);
  fileExtension = NULL;
  filePath = NULL;
}

/**
 * Funció encarregada de la execució del programa.
 * @param socketFd     File Descriptor del servidor.
 * @param configParams Paràmetres de configuració.
 */
void CONNECTION_programExecution(int socketFd, ConfigParams * configParams) {
  char *fileName, *message;
  List readFiles, newFiles;
  Packet packet;

  // Iniciem la execució.
  asprintf(&message, STARTING, configParams->telescopeName);
  write(1, message, strlen(message));
  write(1, CONNECTING_TO, strlen(CONNECTING_TO));

  // Alliberem la memòria demanada.
  free(message);
  message = NULL;

  // Creem la llista de fitxers llegits.
  LIST_create(&readFiles);

  // Enviem nom telescopi
  packet = CONNECTION_buildPacket(NEW_CONNECTION_PROTOCOL, "", (unsigned char*) configParams->telescopeName, 0);
  CONNECTION_sendPacket(socketFd, &packet);

  // Llegim la resposta.
  packet = CONNECTION_unbuildPacket(socketFd);

  // Mirem que la connexió ha sigut satisfactòria
  if (strcmp(packet.header, "CONOK") == 0) {
    // Netegem el paquet.
    CONNECTION_clearPacket(&packet);

    // Connexió establerta.
    write(1, CONNECTION_READY, strlen(CONNECTION_READY));

    while (CONNECTION_checkDisconnection(socketFd, configParams->telescopeName)) {
      // Consultem el directori.
      write(1, TESTING_FILES, strlen(TESTING_FILES));

      // Creem la llista de la consulta, i escanegem el directori.
      LIST_create(&newFiles);
      CONNECTION_scan(configParams->filesDirectory, &readFiles, &newFiles);

      // Ens movem a l'inici de la llista.
      LIST_beginning(&newFiles);

      // Comprovem que no estigui buida la llista.
      if (LIST_empty(newFiles)) {
        write(1, NO_FILES_FOUND, strlen(NO_FILES_FOUND));
      }

      // Consultem la llista dels fitxers no enviats.
      while (!LIST_end(newFiles) && CONNECTION_checkDisconnection(socketFd, configParams->telescopeName) == 1) {
        // Obtenim els detalls del fitxer, i avancem.
        LIST_view(newFiles, &fileName);
        LIST_forward(&newFiles);

        // Comprovem que es tracta d'un fitxer vàlid.
        if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0 && fileName[0] != '.') {
          // Gestionem l'enviament complet del fitxer.
          CONNECTION_sendFileProtocol(configParams, socketFd, fileName);
        }

        // Alliberem la memòria demanada.
        free(fileName);
        fileName = NULL;
      }

      if (!LIST_end(newFiles)) {
        break;
      }

      // Esperem...
      write(1, WAITING, strlen(WAITING));
      LIST_destroy(&newFiles);

      // Esperem un temps per a cada consulta.
      sleep(configParams->waitTime);
    }
  } else {
    // Netegem el paquet.
    CONNECTION_clearPacket(&packet);
  }

  // Sortim de la execució normal del programa.
  asprintf(&message, DISCONNECTING, configParams->telescopeName);
  write(1, message, strlen(message));

  // Alliberem la memòria demanada.
  free(message);
  message = NULL;
}
