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
#ifndef _LIONEL_CONNECTION_H
#define _LIONEL_CONNECTION_H
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <pthread.h>

#define FILES_DIRECTORY "files/%s"
#define FILE_RECEIVING_DATE "%d-%d-%d %d:%d"

// Missatges
#define SOCKET_ERROR "Error! The socket couldn't be initialized.\n"
#define BIND_ERROR "Error! The socket couldn't be binded.\n"
#define FD_ERROR "Error! The FileDescriptor couldn't be created.\n"
#define LISTEN_BACKLOG 64
#define MSG_THREAD_ERROR "Couldn't create the thread\n"
#define MSG_WAIT "Waiting...\n"
#define MSG_DISCONECT "Disconnecting %s...\n"
#define MSG_CON_READY "Connection Lionel-%s ready.\n"
#define MSG_CLIENT_OUT "The client has been disconnected\n"
#define MSG_RCV_IN "Receiving data from %s...\n"
#define MSG_RCV_FP "[%s] %s: %.f%%...\n"
#define MSG_RCV_PRCNT "[%s] Receiving %s... 0%%...\n"
#define MSG_RCV_OUT "It has been a problem douring the transaction. So sorry bro\n"
#define FILE_RECEIVED "File %s received.\n"

// Protocol de comunicació
#define NEW_CONNECTION_PROTOCOL 0x01
#define DISCONNECT_PROTOCOL 0x02
#define RECEIVE_FILE_PROTOCOL 0x03
#define MAX_FRAGMENT 451
#define HEADER_STRUCTURE "[%s]"
#define PACKET_STRUCTURE "%c[%s]%hd[%s]"

// Cues de missatges
#define IMAGE_FILE_TYPE_QUEUE 1
#define TEXT_FILE_TYPE_QUEUE 2
#define STATISTICS_STRUCTURE "\t***** Statistics *****\n\n- Images: %d\n- Text files: %d\n- Total size (in KB): %lf\n- Average constellations: %f\n- Constellations: %d\n- Average density: %f\n- Max magnitude: %f\n- Min magnitude: %f\n\n"

#define MAXHEADER 20
#define MAXDATA 500
#define CODE_MAX 5

// Crides al sistema
#define PAQUITA_PATH "../Paquita/Paquita.c"
#define MD5SUM_SYSTEM_CALL "md5sum %s  > %s.md5"
#define RM_SYSTEM_CALL "rm files/*.md5"

// Kalkun.txt
#define IMAGE_LIST_FILE "Kalkun.txt"
#define TEXT_LIST_FILE "undefined.txt"
#define KALKUN_STRUCTURE "%s %ld bytes\n"

typedef struct _packet {
  char type;
  char * header;
  short length;
  unsigned char * data;
} Packet;

typedef struct {
  unsigned char * fileOwner;
  char * fileName;
  long fileLength;
  long currentLength;
  float percentage;
  char * receivingTime;
} FileProperties;

typedef struct {
  int size;
  int currentIndex;
  int * clientFd;
  pthread_t * clientServers;
} Clients;

// Cues de missatges Lionel - Paquita
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

void CONNECTION_createFileLists();
void CONNECTION_destroyFileLists();
int CONNECTION_connectServer(char * ipAddress, int port);
void CONNECTION_saveImageFilesList();
void* CONNECTION_runServer(void* serverPort);

#endif
