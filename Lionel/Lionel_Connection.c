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
#include "Lionel_Connection.h"
#include "Lionel_List.h"
#include "Lionel_Constellations_List.h"

extern int serverRunning, serverFd, idPaquita;
extern List imagesList, textFilesList;
extern Clients clients;
extern pthread_mutex_t mtx;

/**
 * Funció encarregada d'engegar el servidor.
 * @param  ipAddress Adreça IP del servidor.
 * @param  port      Port del servidor.
 * @return           File Descriptor del socket.
 */
int CONNECTION_connectServer(char * ipAddress, int port) {
  int socketFd;
  struct sockaddr_in sAddr;

  socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  fcntl(socketFd, F_SETFL, O_NONBLOCK);

  if (socketFd < 0) {
    write(1, SOCKET_ERROR, strlen(SOCKET_ERROR));
    return -1;
  }

  bzero(&sAddr, sizeof(sAddr));
  sAddr.sin_family = AF_INET;
  sAddr.sin_port = htons(port);
  sAddr.sin_addr.s_addr = inet_addr(ipAddress);

  if (bind(socketFd, (void *) &sAddr, sizeof(sAddr)) < 0) {
    write(1, BIND_ERROR, strlen(BIND_ERROR));
    close(socketFd);

    return -1;
  }

  listen(socketFd, LISTEN_BACKLOG);

  return socketFd;
}

/**
 * Funció encarregada de gestionar les metadades.
 * @param  data      Metadades rebudes del client.
 * @param  totalSize Tamany total del fitxer a rebre.
 * @return           Nom del fitxer.
 */
char* CONNECTION_manageMetadata(unsigned char* data, long* totalSize){
  char c, *codi, *eType, *secure, *fileName;
  int i = 0, k, in;

  if (data[0] == 'I') {
    eType = (char*) malloc(strlen(".jpg") * sizeof(char));
    strcpy(eType, ".jpg");
  } else {
    eType = (char*) malloc(strlen(".txt") * sizeof(char));
    strcpy(eType, ".txt");
  }

  secure = (char*) malloc(strlen(eType) * sizeof(char));
  strcpy(secure, eType);

  while (data[i] != '&') i++;

  i++;
  *totalSize = 0;
  codi = (char*) malloc(sizeof(char)*10);

  while (data[i] != '&') {
    c = data[i];
    in = (int)c - '0';
    *totalSize = 10 * (*totalSize) + in;
    i++;
  }

  i++;
  k = 0;
  while (data[i] != '\0') {
    codi[k] = data[i];
    i++;
    k++;
  }

  codi[i] = '\0';

  fileName = (char*) malloc(strlen(codi) + strlen(eType) + 1);
  strcpy(fileName, codi);
  strcat(fileName, secure);
  strcat(fileName, "\0");

  // Alliberem la memòria demanada.
  free(codi);
  free(eType);
  free(secure);
  codi = NULL;
  eType = NULL;
  secure = NULL;

  return fileName;
}

/**
 * Funció encarregada de generar el paquet a enviar.
 * @param packet      Paquet a generar.
 * @param operationId Tipus de operació.
 * @param header      Capçalera del paquet.
 * @param data        Dades del paquet.
 * @param argLength   Tamany auxiliar de la informació a enviar.
 */
void CONNECTION_buildPacket(Packet * packet, int operationId, char * header, unsigned char * data, short argLength) {
  int i = 0;

  packet->type = operationId;
  asprintf(&packet->header, HEADER_STRUCTURE, header);

  if (argLength != 0) {
    packet->length = argLength;
  } else {
    packet->length = strlen((char*) data);
  }

  packet->data = (unsigned char*) malloc((packet->length + 1) * sizeof(unsigned char));

  for (i = 0; i < packet->length; i++) {
    packet->data[i] = data[i];
  }

  packet->data[i] = '\0';
}

/**
 * Funció encarregada de llegir el paquet rebut.
 * @param clientFd File Descriptor del client.
 * @param packet   Paquet amb les dades rebudes.
 */
void CONNECTION_unbuildPacket(int clientFd, Packet * packet) {
  int i = 0;
  char aux = ' ', *auxArray;

  // Type
  read(clientFd, &packet->type, sizeof(char));
  read(clientFd, &aux, sizeof(char));           // '['

  // Header
  auxArray = (char*) malloc(sizeof(char));
  read(clientFd, &aux, sizeof(char));

  while (aux != ']') {
    auxArray = (char*) realloc(auxArray, sizeof(auxArray) + sizeof(char));
    auxArray[i] = aux;
    read(clientFd, &aux, sizeof(char));
    i++;
  }

  read(clientFd, &aux, sizeof(char));             // '\0'
  auxArray[i] = '\0';
  packet->header = (char*) malloc(sizeof(auxArray));
  strcpy(packet->header, auxArray);

  // Alliberem la memòria demanada.
  free(auxArray);
  auxArray = NULL;

  // Length
  read(clientFd, &packet->length, sizeof(short));

  //Data
  packet->data = (unsigned char*) malloc((packet->length + 1) * sizeof(unsigned char));

  if (packet->length > 0) {
    read(clientFd, packet->data, packet->length * sizeof(unsigned char));
  }

  read(clientFd, &aux, sizeof(unsigned char));       // '\0'
  packet->data[packet->length] = '\0';
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
  write(clientFd, packet->data, (packet->length + 1) * sizeof(unsigned char));

  // Netegem el paquet.
  CONNECTION_clearPacket(packet);
}

/**
 * Funció encarregada de desar la porció del fitxer al fitxer corresponent.
 * @param filePath    Drecera del fitxer.
 * @param fileContent Porció del fitxer a escriure.
 * @param length      Llargada de la porció del fitxer.
 */
void CONNECTION_storeFilePortion(char * filePath, unsigned char * fileContent, int length) {
  // Obrim el fitxer.
  int fd = open(filePath, O_CREAT | O_WRONLY | O_APPEND);

  if (fd > 0) {
    // Alterem els permissos del fitxer.
    chmod(filePath, S_IRWXU);

    // Movem el cursor al final del contingut.
    lseek(fd, 0, SEEK_END);

    // Afegim el nou contingut al fitxer.
    write(fd, fileContent, length);

    // Tanquem el fitxer.
    close(fd);
  }
}

/**
 * Funció encarregada d'obtenir el Checksum del fitxer indicat.
 * @param  filePath Drecera del fitxer.
 * @return          Checksum MD5 del fitxer.
 */
unsigned char* CONNECTION_computeChecksum(char * filePath) {
  int i, n = 0, pid, pipeOut[2];
  unsigned char aux, *output, *checksum;

  // Creem les dues pipes
  if (pipe(pipeOut) < 0) {
    return 0;
  }

  // Fem el fork.
  pid = fork();

  if (pid > 0) {    // Pare
    // Tanquem un dels ports de la pipe.
    close(pipeOut[1]);

    i = 0;
    output = (unsigned char*) malloc(175 * sizeof(unsigned char));
    checksum = (unsigned char*) malloc(33 * sizeof(unsigned char));

    // Llegim el MD5sum del fill
    while ((n = read(pipeOut[0], &aux, sizeof(unsigned char))) > 0) {
      if (n > 0) {
        output[i] = aux;
        i++;
      }
    }

    // Ens quedem amb la part que ens interessa de l'output.
    for (i = 0; i < 32; i++) {
      checksum[i] = output[i];
    }

    checksum[32] = '\0';

    // Alliberem la memòria demanada.
    free(output);
    output = NULL;

    // Esperem al fill.
    wait(NULL);
  } else {          // Fill
    // Tanquem un dels ports de la pipe.
    close(pipeOut[0]);

    // Dupliquem el File Descriptor de la pantalla.
    dup2(pipeOut[1], 1);

    // Executem la comanda del MD5SUM.
    char *const parmList[] = {"--check", filePath, NULL};
    execv("/usr/bin/md5sum", parmList);

    exit(0);
  }

  // Retornem el Checksum.
  return checksum;
}

/**
 * Funció encarregada de llegir els fitxers de constel·lacions, i desar-ne les dades.
 * @param fProperties        Propietats del fitxer.
 * @param constellationsList Llistat de les constel·lacions.
 */
void CONNECTION_parseFileContent(FileProperties fProperties, ConstellationsList * constellationsList) {
  int j, fd, i;
  long size, currentSize;
  char *pa, fileContent, **info;

  asprintf(&pa, FILES_DIRECTORY, fProperties.fileName);
  fd = open(pa, O_RDONLY);

  info = (char**) malloc(3 * sizeof(char*));

  // Allibero la memòria demanada.
  free(pa);
  pa = NULL;

  if (fd < 0) {
    write(1, FD_ERROR, strlen(FD_ERROR));
  } else {
    size = fProperties.fileLength;
    currentSize = 0;

    for (i = 0; i < 3; i++) {
      info[i] =(char*) malloc(sizeof(char));
    }

    i = 0;

    while (currentSize < size) {
      read(fd, &fileContent, sizeof(char));
      currentSize++;

      if (fileContent == ' '){
        info[i][j] = '\0';
        i++;
        j = 0;
      } else if (fileContent == '\n' || fileContent == '\0') {
        info[i][j] = '\0';
        CONSTELLATIONS_LIST_add(constellationsList, info[0], atof(info[1]), atof(info[2]));

        for (i = 0; i < 3; i++) {
          free(info[i]);
          info[i] = NULL;
        }

        for (i = 0; i < 3; i++){
          info[i]=(char*) malloc(sizeof(char));
        }

        i = 0;
        j = 0;
      } else {
        info[i] = (char*) realloc(info[i], sizeof(info[i]) + sizeof(char));
        info[i][j] = fileContent;
        j++;
      }
    }

    // Alliberem la memòria demanada.
    free(info);
    info = NULL;

    // Tanquem el fitxer.
    close(fd);
  }
}

/**
 * Funció encarregada de controlar el percentatge actual del fitxer rebut.
 * @param fileOwner  Propietari del fitxer.
 * @param fileName   Nom del fitxer.
 * @param percentage Percentatge del fitxer rebut.
 */
void CONNECTION_currentFileSize(char * fileOwner, char * fileName, float percentage) {
  char * message;

  // Construim el missatge.
  asprintf(&message, MSG_RCV_FP, fileOwner, fileName, percentage);

  // Mostrem per pantalla el missatge.
  write(1, message, strlen(message) * sizeof(char));

  // Alliberem la memòria demanada.
  free(message);
  message = NULL;
}

/**
 * Funció encarregada d'obtenir les estadístiques de Paquita.
 */
void CONNECTION_getStatistics() {
  char * message;
  Message * mPaquita = (Message*) malloc(sizeof(Message));

  // Rebem la resposta.
  msgrcv(idPaquita, mPaquita, sizeof(Message), 0, 0);

  if (mPaquita->queueType == FEEDBACK_TYPE_QUEUE) {
    asprintf(&message, STATISTICS_STRUCTURE, mPaquita->statistics.imageFiles, mPaquita->statistics.textFiles,
      mPaquita->statistics.totalSize, mPaquita->statistics.averageConstellations, mPaquita->statistics.constellationsNumber,
      mPaquita->statistics.averageDensity, mPaquita->statistics.maxMagnitude, mPaquita->statistics.minMagnitude);

    write(1, message, strlen(message) * sizeof(char));

    // Alliberem la memòria demanada.
    free(message);
    message = NULL;
  }

  // Alliberem la memòria demanada.
  free(mPaquita);
  mPaquita = NULL;
}

/**
 * Funció encarregada de notificar a Paquita sobre un fitxer.
 * @param fileType Tipus de fitxer.
 * @param fileSize Tamany del fitxer.
 */
void CONNECTION_fileNotification(int fileType, long fileSize) {
  Message * message = (Message*) malloc(sizeof(Message));

  // Definim les dades.
  message->queueType = FILES_TYPE_QUEUE;
  message->killPaquita = 0;
  message->fMessage.fileType = fileType;
  message->fMessage.fileSize = fileSize;

  // Enviem la dada.
  msgsnd(idPaquita, message, sizeof(Message), IPC_NOWAIT);

  // Obtenem les estadístiques.
  CONNECTION_getStatistics();

  // Alliberem la memòria demanada.
  free(message);
  message = NULL;
}

/**
 * Funció encarregada d'enviar les constel·lacions a Paquita.
 * @param cList [description]
 */
void CONNECTION_sendConstellations(ConstellationsList * cList) {
  char * code = (char*) malloc(CODE_MAX * sizeof(char));
  double density, magnitude;
  Message * message = (Message*) malloc(sizeof(Message));

  // Anem a l'inici de la llista.
  CONSTELLATIONS_LIST_beginning(cList);

  // Mentre quedin constel·lacions a enviar.
  while (!CONSTELLATIONS_LIST_end(*cList)) {
    // Obtenim i preparem les dades a enviar.
    CONSTELLATIONS_LIST_view(*cList, code, &density, &magnitude);
    strcpy(message->cMessage.code, code);
    message->cMessage.density = (float) density;
    message->cMessage.magnitude = (float) magnitude;
    message->queueType = CONSTELLATIONS_TYPE_QUEUE;

    // Enviem la dada.
    msgsnd(idPaquita, message, sizeof(Message), IPC_NOWAIT);

    // Obtenem les estadístiques.
    CONNECTION_getStatistics();

    // Avancem en la llista.
    CONSTELLATIONS_LIST_forward(cList);
  }

  // Alliberem la memòria demanada.
  free(code);
  free(message);
  code = NULL;
  message = NULL;
}

/**
 * Funció encarregada de gestionar una nova connexió.
 * @param received Paquet rebut.
 * @param clientFd File Descriptor del client.
 */
void CONNECTION_newConnectionProtocol(Packet * received, int clientFd) {
  char * buffer;
  Packet packet;

  // Notifiquem sobre la connexió establerta.
  asprintf(&buffer, MSG_CON_READY, received->data);
  write(1, buffer, strlen(buffer));

  // Enviem el paquet de resposta.
  CONNECTION_buildPacket(&packet, NEW_CONNECTION_PROTOCOL, "CONOK", (unsigned char*) "", 0);
  CONNECTION_sendPacket(clientFd, &packet);

  write(1, MSG_WAIT, strlen(MSG_WAIT));

  // Alliberem la memòria demanada.
  free(buffer);
  buffer = NULL;
}

/**
 * Funció encarregada de gestionar una desconnexió.
 * @param fProperties   Propietats del fitxer.
 * @param clientFd      File Descriptor del client.
 * @param clientRunning Flag encarregat de marcar que el client es troba en execució.
 */
void CONNECTION_disconnectProtocol(FileProperties * fProperties, int clientFd, int * clientRunning) {
  char *buffer;
  Packet packet;

  // Si els tamanys són correctes...
  if (fProperties->currentLength == fProperties->fileLength) {
    // Donem feedback al client.
    CONNECTION_buildPacket(&packet, DISCONNECT_PROTOCOL, "CONOK", (unsigned char*) "", 0);
    CONNECTION_sendPacket(clientFd, &packet);

    // Informem de la desconnexió.
    asprintf(&buffer, MSG_DISCONECT, fProperties->fileOwner);
    write(1, buffer, strlen(buffer));

    // Alliberem la memòria demanada.
    free(buffer);
    buffer = NULL;

    // Acabem la execució del servidor dedicat.
    *clientRunning = 0;
  } else {
    CONNECTION_buildPacket(&packet, DISCONNECT_PROTOCOL, "CONKO", (unsigned char*) "", 0);
    CONNECTION_sendPacket(clientFd, &packet);
  }
}

/**
 * Funció encarregada de gestionar el paquet de metadades.
 * @param received    Paquet rebut.
 * @param fProperties Propietats del fitxer.
 * @param clientFd    File Descriptor del client.
 */
void CONNECTION_manageMetadataPacket(Packet * received, FileProperties * fProperties, int clientFd) {
  char *buffer;
  Packet packet;

  // Notifiquem sobre el propietari del fitxer.
  asprintf(&buffer, MSG_RCV_IN, fProperties->fileOwner);
  write(1, buffer, strlen(buffer));

  // Desem les dades del fitxer.
  //fProperties->fileName = (char*) malloc(sizeof(char));
  fProperties->fileName = CONNECTION_manageMetadata(received->data, &fProperties->fileLength);
  fProperties->currentLength = 0;

  // Alliberem la memòria demanada.
  free(buffer);
  buffer = NULL;

  asprintf(&buffer, MSG_RCV_PRCNT, (char*) fProperties->fileOwner, fProperties->fileName);
  write(1, buffer, strlen(buffer));

  // Alliberem la memòria demanada.
  free(buffer);
  buffer = NULL;

  fProperties->percentage = 0;

  // Notifiquem de l'estat de l'enviament.
  CONNECTION_buildPacket(&packet, RECEIVE_FILE_PROTOCOL, "FILEOK", (unsigned char*) "", 0);
  CONNECTION_sendPacket(clientFd, &packet);
}

/**
 * Funció encarregada de gestionar un paquet de dades de fitxer.
 * @param received    Paquet rebut.
 * @param fProperties Propietats del fitxer.
 * @param clientFd    File Descriptor del client.
 */
void CONNECTION_manageFilePacket(Packet * received, FileProperties * fProperties, int clientFd) {
  char *filePath;
  float percentage;
  Packet packet;

  // Obtenim la drecera del fitxer.
  asprintf(&filePath, FILES_DIRECTORY, fProperties->fileName);

  // Actualitzem el percentatge del fitxer.
  percentage = (fProperties->currentLength * 100) / fProperties->fileLength;

  // Si són diferents...
  if (percentage != fProperties->percentage) {
    // Mostrem per pantalla l'estat actual d'enviament del fitxer, i actualitzem el valor.
    CONNECTION_currentFileSize((char*) fProperties->fileOwner, fProperties->fileName, percentage);
    fProperties->percentage = percentage;
  }

  fProperties->currentLength += received->length;
  CONNECTION_storeFilePortion(filePath, received->data, received->length);

  // Notifiquem de l'estat de l'enviament.
  CONNECTION_buildPacket(&packet, RECEIVE_FILE_PROTOCOL, "FILEOK", (unsigned char*) "", 0);
  CONNECTION_sendPacket(clientFd, &packet);

  // Alliberem la memòria demanada.
  free(filePath);
  filePath = NULL;
}

/**
 * Funció encarregada de gestionar el paquet de final d'enviament de fitxer.
 * @param received    Paquet rebut.
 * @param fProperties Propietats del fitxer.
 * @param clientFd    File Descriptor del client.
 */
void CONNECTION_manageEndOfFilePacket(Packet * received, FileProperties * fProperties, int clientFd) {
  int j, tDay, tHour, tYear, tMonth, tMinutes, checksumSize;
  char *buffer, *filePath;
  time_t rawtime;
  Packet packet;
  struct tm * timeinfo;
  unsigned char *lionelChecksum;
  ConstellationsList constellationsList;

  // Obtenim la drecera del fitxer.
  asprintf(&filePath, FILES_DIRECTORY, fProperties->fileName);

  CONNECTION_currentFileSize((char*) fProperties->fileOwner, fProperties->fileName, 100);
  lionelChecksum = CONNECTION_computeChecksum(filePath);
  checksumSize = sizeof(lionelChecksum);

  if (memcmp(lionelChecksum, received->data, checksumSize) == 0) {
    CONNECTION_buildPacket(&packet, RECEIVE_FILE_PROTOCOL, "CHECKOK", (unsigned char*) "", 0);

    for (j = 0; (fProperties->fileName)[j] != '.'; j++);

    // Controlem la hora d'enviament.
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    tYear = timeinfo->tm_year + 1900;
    tMonth = timeinfo->tm_mon + 1;
    tDay = timeinfo->tm_mday;
    tHour = timeinfo->tm_hour + 1;
    tMinutes = timeinfo->tm_min;

    // Formatem la hora d'enviament.
    fProperties->receivingTime = malloc(sizeof(FILE_RECEIVING_DATE));
    sprintf(fProperties->receivingTime, FILE_RECEIVING_DATE, tYear, tMonth, tDay, tHour, tMinutes);

    // Controlem que es tracta d'una imatge.
    if ((fProperties->fileName)[j + 1] == 'j' || (fProperties->fileName)[j + 1] == 'J') {
      pthread_mutex_lock(&mtx);

      // Afegim la dada a la llista.
      LIST_add(&imagesList, fProperties->receivingTime, fProperties->fileLength);

      // Informem a Paquita (señora, niña bonita) de la imatge.
      CONNECTION_fileNotification(IMAGE_FILE_TYPE_QUEUE, fProperties->fileLength);

      pthread_mutex_unlock(&mtx);
    } else {
      pthread_mutex_lock(&mtx);

      // Creem la llista de les constel·lacions.
      CONSTELLATIONS_LIST_create(&constellationsList);

      // Obtenim les constel·lacions i les afegim a la llista.
      CONNECTION_parseFileContent(*fProperties, &constellationsList);
      LIST_add(&textFilesList, fProperties->receivingTime, fProperties->fileLength);

      // Informem a Paquita (señora, niña bonita) del fitxer de text.
      CONNECTION_fileNotification(TEXT_FILE_TYPE_QUEUE, fProperties->fileLength);

      // Informem a Paquita (señora, niña bonita) de les constel·lacions del fitxer de text.
      CONNECTION_sendConstellations(&constellationsList);

      // Destruim la llista de les constel·lacions.
      CONSTELLATIONS_LIST_destroy(&constellationsList);

      pthread_mutex_unlock(&mtx);
    }
  } else {
    CONNECTION_buildPacket(&packet, RECEIVE_FILE_PROTOCOL, "CHECKKO", (unsigned char*) "", 0);
  }

  // Enviem la resposta al client.
  CONNECTION_sendPacket(clientFd, &packet);

  // Notifiquem del fitxer.
  asprintf(&buffer, FILE_RECEIVED, fProperties->fileName);
  write(1, buffer, strlen(buffer));

  // Esperem...
  write(1, MSG_WAIT, strlen(MSG_WAIT));

  // Alliberem la memòria demanada.
  free(buffer);
  free(filePath);
  free(lionelChecksum);
  buffer = NULL;
  filePath = NULL;
  lionelChecksum = NULL;
}

/**
 * Funció encarregada de gestionar el paquet rebut.
 * @param received      Paquet rebut.
 * @param clientFd      File Descriptor del client.
 * @param fProperties   Propietats del fitxer enviat.
 * @param clientRunning Flag d'execució del client.
 */
void CONNECTION_managePacket(Packet * received, int clientFd, FileProperties * fProperties, int * clientRunning) {
  switch (received->type) {
    case NEW_CONNECTION_PROTOCOL:
      CONNECTION_newConnectionProtocol(received, clientFd);
      break;

    case DISCONNECT_PROTOCOL:
      CONNECTION_disconnectProtocol(fProperties, clientFd, clientRunning);
      break;

    case RECEIVE_FILE_PROTOCOL:
      // Controlem la metadata del fitxer.
      if (strcmp(received->header, "METADATA") == 0) {
        CONNECTION_manageMetadataPacket(received, fProperties, clientFd);
      }

      // Controlem l'enviament del fitxer.
      if (strcmp(received->header, "") == 0) {
        CONNECTION_manageFilePacket(received, fProperties, clientFd);
      }

      // Controlem el final de l'enviament del fitxer.
      if (strcmp(received->header, "ENDFILE") == 0) {
        CONNECTION_manageEndOfFilePacket(received, fProperties, clientFd);
      }
      break;
  }

  // Esborrem el paquet rebut.
  CONNECTION_clearPacket(received);
}

/**
 * Funció encarregada de córrer el servidor dedicat a un client.
 * @param arg File Descriptor del client.
 */
void* CONNECTION_clients(void *arg) {
  int clientRunning = 1, clientFd = *((int *) arg);
  Packet packet;
  FileProperties fProperties;

  // Llegim la trama de nova connexió.
  CONNECTION_unbuildPacket(clientFd, &packet);
  fProperties.fileOwner = (unsigned char*) malloc(packet.length * sizeof(unsigned char));
  memcpy(fProperties.fileOwner, packet.data, packet.length * sizeof(unsigned char));
  CONNECTION_managePacket(&packet, clientFd, &fProperties, &clientRunning);

  // Mentre el client estigui fent jogging...
  while (clientRunning == 1) {
    CONNECTION_unbuildPacket(clientFd, &packet);
    CONNECTION_managePacket(&packet, clientFd, &fProperties, &clientRunning);
  }

  // Tanquem el client.
  close(clientFd);

  return (void*) 0;
}

/**
 * Funció encarregada d'actualitzar el fitxer amb els fitxers rebuts.
 * @param file      Drecera del fitxer a desar.
 * @param filesList Llista amb els fitxers a desar.
 */
void CONNECTION_saveFilesList(char * file, List filesList) {
  int fd;
  long fileSize;
  char *date, *text;

  // Obrim el fitxer. Si no està creat, el creem.
  fd = open(file, O_CREAT | O_WRONLY | O_APPEND);

  if (fd > 0) {
    // Alterem els permissos del fitxer.
    chmod(file, S_IRWXU);

    // Afegim el nou contingut al fitxer.
    lseek(fd, 0, SEEK_END);

    // Anem a l'inici de la llista.
    LIST_beginning(&filesList);

    // Mentre hi ha elements...
    while (!LIST_end(filesList)) {
      // Consultem la dada en la posició actual.
      LIST_view(filesList, &date, &fileSize);

      // Generem el missatge, i l'escrivim en el fitxer.
      asprintf(&text, FILES_LIST_STRUCTURE, date, fileSize);
      write(fd, text, strlen(text));

      // Avancem el cursor de la llista.
      LIST_forward(&filesList);

      // Alliberem la memòria demanada.
      free(date);
      free(text);
      date = NULL;
      text = NULL;
    }

    // Tanquem el fitxer.
    close(fd);
  }
}

/**
 * Funció encarregada de córrer el servidor.
 * @param mainRun Flag encarregat de marcar que Lionel es troba executant-se.
 */
void* CONNECTION_runServer(void * mainRun) {
  int j, i = 0, c = sizeof(struct sockaddr_in), clientFd, *clientsFd = (int*) malloc(sizeof(int)),
      *isRunning = (int*) mainRun;
  pthread_t * threads_id = (pthread_t*) malloc(sizeof(pthread_t));
  struct sockaddr_in client;

  write(1, MSG_WAIT, strlen(MSG_WAIT));

  // Mentre el servidor estigui corrent...
  while (serverRunning == 1) {
    clientFd = -1;
    clientFd = accept(serverFd, (struct sockaddr *) &client, (socklen_t*) &c);

    if (clientFd > 0) {
      clientsFd[i] = clientFd;
      clientsFd = (int*) realloc(clientsFd, sizeof(clientsFd) + sizeof(int));
      threads_id = (pthread_t*) realloc(threads_id, sizeof(threads_id) + sizeof(pthread_t));
      pthread_create(&threads_id[i], NULL, CONNECTION_clients, &clientsFd[i]);
      i++;
    }
  }

  // Tanquem tots els threads.
  for (j = 0; j < i; j++) {
    pthread_join(threads_id[i], NULL);
  }

  // Actualitzem el fitxers amb les imatges i els fitxers de text rebuts.
  CONNECTION_saveFilesList(IMAGE_LIST_FILE, imagesList);
  CONNECTION_saveFilesList(TEXT_LIST_FILE, textFilesList);

  // Alliberem la memòria demanada.
  free(clientsFd);
  free(threads_id);
  clientsFd = NULL;
  threads_id = NULL;

  // Tanquem Lionel.
  *isRunning = 0;

	return (void*) 0;
}
