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

extern int serverRunning, serverFd;
extern Clients clients;
extern List imagesList, textFilesList;
extern pthread_mutex_t mtx;
extern int idFile, idConstellation, idFeedback;

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
 * @param packet      Paquet a preparar.
 * @param operationId Tipus de operació.
 * @param header      Capçalera del paquet.
 * @param data        Dades del paquet.
 * @param argLength   Tamany auxiliar de la informació a enviar.
 */
void CONNECTION_buildPacket(Packet * packet, int operationId, char * header, unsigned char * data, short argLength) {
  int i = 0, size;

  size = sizeof(packet->data);
  strcpy(packet->header, "");
  memset(packet->data, 0, size);

  packet->type = operationId;
  //asprintf(&packet.header, HEADER_STRUCTURE, header);
  sprintf(packet->header, HEADER_STRUCTURE, header);

  if (argLength != 0) {
    packet->length = argLength;
  } else {
    packet->length = strlen((char*) data);
  }

  for (i = 0; i < packet->length; i++) {
    packet->data[i] = data[i];
  }

  packet->data[i] = '\0';
}

/**
 * Funció encarregada de llegir el paquet rebut.
 * @param  clientFd File Descriptor del client.
 * @return          Paquet amb les dades rebudes.
 */
void CONNECTION_unbuildPacket(int clientFd, Packet * packet) {
  int i = 0, size;
  char aux = ' ', *auxArray;

/*
  // Netegem el paquet.
  size = sizeof(*packet);
  memset(packet, 0, size);*/

  size = sizeof(packet->data);
  strcpy(packet->header, "");
  memset(packet->data, 0, size);

  // Type
  read(clientFd, &packet->type, sizeof(char));
  read(clientFd, &aux, sizeof(char));           // '['

  // Header
  auxArray = (char*) malloc(sizeof(char) * MAXHEADER);
  read(clientFd, &aux, sizeof(char));

  while (aux != ']') {
    //auxArray = (char*) realloc(auxArray, size * sizeof(char));
    auxArray[i] = aux;
    read(clientFd, &aux, sizeof(char));
    i++;
  }

  read(clientFd, &aux, sizeof(char));             // '\0'
  auxArray[i] = '\0';
  //packet.header = (char*) malloc(sizeof(auxArray));
  strcpy(packet->header, auxArray);

  // Alliberem la memòria demanada.
  free(auxArray);
  auxArray = NULL;

  // Length
  read(clientFd, &packet->length, sizeof(short));

  //Data
  //packet.data = (unsigned char*) malloc((packet.length + 1) * sizeof(unsigned char));

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
  packet->header = NULL;

  //int size = sizeof(packet->data);
  //packet->data = (unsigned char*) realloc(packet->data, 0);
  free(packet->data);
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
  //CONNECTION_clearPacket(packet);
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
       i++;
       read(fd, &aux, sizeof(unsigned char));
     }

     checksum[i] = '\0';
   }

   // Alliberem la memòria demanada.
   free(systemCall);
   free(auxFilePath);
   systemCall = NULL;
   auxFilePath = NULL;

   // Esborrem els fitxers no desitjats.
   system(RM_SYSTEM_CALL);

   return checksum;
}

void CONNECTION_parseFileContent(FileProperties fProperties, ConstellationsList * constellationsList) {
  int j, fd, i;
  long size, currentSize;
  char *pa, fileContent, **marialluisa;

  asprintf(&pa, FILES_DIRECTORY, fProperties.fileName);
  fd = open(pa, O_RDONLY);

  marialluisa = (char**) malloc(3 * sizeof(char*));

  // Allibero la memòria demanada.
  free(pa);
  pa = NULL;

  if (fd < 0) {
    write(1, FD_ERROR, strlen(FD_ERROR));
  } else {
    size = fProperties.fileLength;
    currentSize=0;

    for (i = 0; i < 3; i++) {
      marialluisa[i] =(char*) malloc(sizeof(char));
    }
    i = 0;

    while (currentSize < size) {
      read(fd, &fileContent, sizeof(char));
      currentSize++;
      if(fileContent==' '){
        marialluisa[i][j]='\0';
        i++;
        j=0;
      }else if (fileContent=='\n' || fileContent=='\0'){
        marialluisa[i][j]='\0';
        CONSTELLATIONS_LIST_add(constellationsList, marialluisa[0], atof(marialluisa[1]), atof(marialluisa[2]));
        for(i=0; i<3; i++){
          free(marialluisa[i]);
          marialluisa[i]=NULL;
        }
        for(i=0; i<3; i++){
          marialluisa[i]=(char*) malloc(sizeof(char));
        }
        i=0;
        j=0;
      }else{
        printf("hola\n");
        marialluisa[i] = (char*) realloc(marialluisa[i], sizeof(marialluisa[i])+sizeof(char));
        printf("adeu\n");
        marialluisa[i][j] = fileContent;
        j++;
      }
    }

    free(marialluisa);
    marialluisa=NULL;

    // Tanco el fitxer.
    close(fd);
  }
}

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

void CONNECTION_getStatistics() {
  char * message;
  FeedbackMessage * feedMessage = (FeedbackMessage*) malloc(sizeof(FeedbackMessage));

  // Rebem la resposta.
  msgrcv(idFeedback, feedMessage, sizeof(FeedbackMessage), 0, 0);

  asprintf(&message, STATISTICS_STRUCTURE, feedMessage->imageFiles, feedMessage->textFiles,
    feedMessage->totalSize, feedMessage->averageConstellations, feedMessage->constellationsNumber,
    feedMessage->averageDensity, feedMessage->maxMagnitude, feedMessage->minMagnitude);

  write(1, message, strlen(message) * sizeof(char));

  // Alliberem la memòria demanada.
  free(feedMessage);
  free(message);
  feedMessage = NULL;
  message = NULL;
}

void CONNECTION_fileNotification(int fileType, long fileSize) {
  FileMessage * fMessage = (FileMessage*) malloc(sizeof(FileMessage));

  fMessage->fileType = fileType;
  fMessage->fileSize = fileSize;

  // Enviem la dada.
  msgsnd(idFile, fMessage, sizeof(FileMessage), IPC_NOWAIT);

  // Obtenem les estadístiques.
  CONNECTION_getStatistics();

  // Alliberem la memòria demanada.
  free(fMessage);
  fMessage = NULL;
}

void CONNECTION_sendConstellations(ConstellationsList * cList) {

}

void CONNECTION_preparePacket(Packet * packet) {
  packet->header = (char*) malloc(MAXHEADER * sizeof(char));
  packet->data = (unsigned char*) malloc(MAXDATA * sizeof(unsigned char));
}

void CONNECTION_managePacket(Packet * received, Packet * packet, int clientFd, FileProperties * fProperties, int * clientRunning) {
  int j, tDay, tHour, tYear, tMonth, tMinutes, checksumSize;
  char *filePath, *buffer;
  float percentage;
  time_t rawtime;
  struct tm * timeinfo;
  unsigned char * lionelChecksum;
  ConstellationsList constellationsList;

  // Obtenim la drecera del fitxer.
  asprintf(&filePath, FILES_DIRECTORY, fProperties->fileName);

  printf("managePacket\n");

  switch (received->type) {
    case NEW_CONNECTION_PROTOCOL:
      asprintf(&buffer, MSG_CON_READY, received->data);
      write(1, buffer, strlen(buffer));

      CONNECTION_buildPacket(packet, NEW_CONNECTION_PROTOCOL, "CONOK", (unsigned char*) "", 0);
      CONNECTION_sendPacket(clientFd, packet);

      write(1, MSG_WAIT, strlen(MSG_WAIT));

      // Alliberem la memòria demanada.
      free(buffer);
      buffer = NULL;
      break;

    case DISCONNECT_PROTOCOL:
      if (fProperties->currentLength == fProperties->fileLength) {
        // Donem feedback al client.
        CONNECTION_buildPacket(packet, DISCONNECT_PROTOCOL, "CONOK", (unsigned char*) "", 0);
        CONNECTION_sendPacket(clientFd, packet);

        // Informem de la desconnexió.
        asprintf(&buffer, MSG_DISCONECT, fProperties->fileOwner);
        write(1, buffer, strlen(buffer));

        // Alliberem la memòria demanada.
        free(buffer);
        buffer = NULL;

        // Acabem la execució del servidor dedicat.
        *clientRunning = 0;
        printf("Nena, deixo l'spinning...\n");
      } else {
        CONNECTION_buildPacket(packet, DISCONNECT_PROTOCOL, "CONKO", (unsigned char*) "", 0);
        CONNECTION_sendPacket(clientFd, packet);
      }
      break;

    case RECEIVE_FILE_PROTOCOL:
      // Controlem la metadata del fitxer.
      if (strcmp(received->header, "METADATA") == 0) {
        printf("METADATA\n");
        // Notifiquem sobre el propietari del fitxer.
        asprintf(&buffer, MSG_RCV_IN, fProperties->fileOwner);
        write(1, buffer, strlen(buffer));

        // Desem les dades del fitxer.
        fProperties->fileName = (char*) malloc(sizeof(char));
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
        CONNECTION_buildPacket(packet, RECEIVE_FILE_PROTOCOL, "FILEOK", (unsigned char*) "", 0);
        CONNECTION_sendPacket(clientFd, packet);
        printf("outtt5\n");
      }

      // Controlem l'enviament del fitxer.
      if (strcmp(received->header, "") == 0) {
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
        CONNECTION_buildPacket(packet, RECEIVE_FILE_PROTOCOL, "FILEOK", (unsigned char*) "", 0);
        CONNECTION_sendPacket(clientFd, packet);
      }

      // Controlem el final de l'enviament del fitxer.
      if (strcmp(received->header, "ENDFILE") == 0) {
        CONNECTION_currentFileSize((char*) fProperties->fileOwner, fProperties->fileName, 100);
        lionelChecksum = CONNECTION_computeChecksum(filePath);
        checksumSize = sizeof(lionelChecksum);

        if (memcmp(lionelChecksum, received->data, checksumSize) == 0) {
          printf("NENAAAAAAA!!!! Que sois igualeeeeeeesss!!!\n");
          CONNECTION_buildPacket(packet, RECEIVE_FILE_PROTOCOL, "CHECKOK", (unsigned char*) "", 0);

          /*
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
            //CONNECTION_fileNotification(IMAGE_FILE_TYPE_QUEUE, fProperties->fileLength);

            pthread_mutex_unlock(&mtx);
          } else {
            pthread_mutex_lock(&mtx);

            // Creem la llista de les constel·lacions.
            CONSTELLATIONS_LIST_create(&constellationsList);

            // Obtenim les constel·lacions i les afegim a la llista.
            CONNECTION_parseFileContent(*fProperties, &constellationsList);
            LIST_add(&textFilesList, fProperties->receivingTime, fProperties->fileLength);

            // Informem a Paquita (señora, niña bonita) del fitxer de text.
            //CONNECTION_fileNotification(TEXT_FILE_TYPE_QUEUE, fProperties->fileLength);

            // Informem a Paquita (señora, niña bonita) de les constel·lacions del fitxer de text.
            //CONNECTION_sendConstellations(&constellationsList);

            // Destruim la llista de les constel·lacions.
            CONSTELLATIONS_LIST_destroy(&constellationsList);

            pthread_mutex_unlock(&mtx);
          }*/
        } else {
          CONNECTION_buildPacket(packet, RECEIVE_FILE_PROTOCOL, "CHECKKO", (unsigned char*) "", 0);
        }

        // Enviem la resposta al client.
        CONNECTION_sendPacket(clientFd, packet);

        // Notifiquem del fitxer.
        asprintf(&buffer, FILE_RECEIVED, fProperties->fileName);
        write(1, buffer, strlen(buffer));

        // Esperem...
        write(1, MSG_WAIT, strlen(MSG_WAIT));

        // Alliberem la memòria demanada.
        free(buffer);
        buffer = NULL;
      }
      break;
  }

  // Alliberem la memòria demanada.
  free(filePath);
  filePath = NULL;
  printf("lluisIPol\n");
}

/**
 * Funció encarregada d'actualitzar el fitxer amb les imatges rebudes.
 */
void CONNECTION_saveImageFilesList() {
  int fd;
  long fileSize;
  char *date, *text;

  // Obrim el fitxer. Si no està creat, el creem.
  fd = open(IMAGE_LIST_FILE, O_CREAT | O_WRONLY | O_APPEND);

  if (fd > 0) {
    // Alterem els permissos del fitxer.
    chmod(IMAGE_LIST_FILE, S_IRWXU);

    // Afegim el nou contingut al fitxer.
    lseek(fd, 0, SEEK_END);

    // Anem a l'inici de la llista.
    LIST_beginning(&imagesList);

    // Mentre hi ha elements...
    while (!LIST_end(imagesList)) {
      // Consultem la dada en la posició actual.
      LIST_view(imagesList, &date, &fileSize);

      // Generem el missatge, i l'escrivim en el fitxer.
      asprintf(&text, KALKUN_STRUCTURE, date, fileSize);
      write(fd, text, strlen(text));

      // Avancem el cursor de la llista.
      LIST_forward(&imagesList);

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
 * Funció encarregada de córrer el servidor dedicat a un client.
 * @param arg File Descriptor del client.
 */
void* CONNECTION_clients(void *arg) {
  int clientRunning = 1, clientFd = *((int *) arg);
  Packet packet, sender;
  FileProperties fProperties;

  CONNECTION_preparePacket(&packet);
  CONNECTION_preparePacket(&sender);

  // Llegim la trama de nova connexió.
  CONNECTION_unbuildPacket(clientFd, &packet);
  fProperties.fileOwner = (unsigned char*) malloc(packet.length * sizeof(unsigned char));
  memcpy(fProperties.fileOwner, packet.data, packet.length * sizeof(unsigned char));
  CONNECTION_managePacket(&packet, &sender, clientFd, &fProperties, &clientRunning);

  // Mentre el client estigui fent jogging...
  while (clientRunning == 1) {
    printf("ElPapaDeRomaEsGay\n");
    CONNECTION_unbuildPacket(clientFd, &packet);
    CONNECTION_managePacket(&packet, &sender, clientFd, &fProperties, &clientRunning);
  }

  // Netegem el paquet rebut.
  CONNECTION_clearPacket(&packet);
  CONNECTION_clearPacket(&sender);

  // Tanquem el client.
  close(clientFd);

  return (void*) 0;
}

/**
 * Funció encarregada de córrer el servidor.
 * @param server File Descriptor del servidor.
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
      clientsFd = (int*) realloc(clientsFd, sizeof(clientFd) + sizeof(int));
      threads_id = (pthread_t*) realloc(threads_id, sizeof(threads_id) + sizeof(pthread_t));
      pthread_create(&threads_id[i], NULL, CONNECTION_clients, &clientsFd[i]);
      i++;
    }
  }

  for (j = 0; j < i; j++) {
    pthread_join(threads_id[i], NULL);
  }

  // Actualitzem el fitxer amb les imatges rebudes.
  CONNECTION_saveImageFilesList();

  // TODO: Fitxer amb els fitxers de text.

  // Alliberem la memòria demanada.
  free(clientsFd);
  free(threads_id);
  clientsFd = NULL;
  threads_id = NULL;

  *isRunning = 0;

	return (void*) 0;
}
