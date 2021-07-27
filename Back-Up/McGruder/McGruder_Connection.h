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
#ifndef _MCGRUDER_CONNECTION_H
#define _MCGRUDER_CONNECTION_H
#define _OPEN_SYS_ITOA_EXT
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
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
#include <dirent.h>
#include <pthread.h>
#include <math.h>

#include "McGruder_Configuration.h"
#include "McGruder_List.h"

// Fitxers
#define FILES_DIRECTORY "%s/%s"
#define FILE_CREATION_DATE "%d%d%d%d%d%d"
#define FILE_TEXT_TYPE "Text File"
#define FILE_IMAGE_TYPE "Image File"
#define FILE_METADATA "%s&%d&%s"
#define IMAGE_FILE_TYPE 1
#define TEXT_FILE_TYPE 2
#define MAX_FRAGMENT 450
#define JPG_LOWER_FILE_FORMAT "jpg"
#define JPG_UPPER_FILE_FORMAT "JPG"
#define JPEG_LOWER_FILE_FORMAT "jpeg"
#define JPEG_UPPER_FILE_FORMAT "JPEG"
#define TXT_LOWER_FILE_FORMAT "txt"
#define TXT_UPPER_FILE_FORMAT "TXT"
//#define SPLITTER 100

// Trama
#define NEW_CONNECTION_PROTOCOL 0x01
#define DISCONNECT_PROTOCOL 0x02
#define SEND_FILE_PROTOCOL 0x03
#define HEADER_STRUCTURE "[%s]"
#define PACKET_STRUCTURE "%c[%s]%hd[%s]"

// Missatges
#define SOCKET_ERROR "Error! The socket couldn't be initialized.\n"
#define CONNECTION_ERROR "Error! The connection couldn't be done.\n"
#define CONNECTING_TO "Connecting to Lionel...\n"
#define CONNECTION_READY "Connection ready.\n"
#define STARTING "Starting %s.\n"
#define WAITING "Waiting...\n"
#define TESTING_FILES "Testing files...\n"
#define FILE_ERROR "No files found.\n"
#define NO_FILES_FOUND "No files found.\n"
#define ERROR_OPENING_FILE "Error! The file couldn't be opened.\n"
#define FILE_NAME "File: %s\n"
#define SENDING "Sending %s...\n"
#define SENDING_ERROR "Error sending file.\n"
#define FILE_SENT "File sent.\n"
#define FILE_SENT_ERROR "File sent incorrectly.\n"
#define DISCONNECTING "Disconnecting %s.\n"

#define MD5SUM_SYSTEM_CALL "md5sum %s  > %s.md5"
#define RM_SYSTEM_CALL "rm %s"

#define MAXHEADER 30
#define MAXCREATIONDATE 30

typedef struct _packet {
  char type;
  char * header;
  short length;
  unsigned char * data;
} Packet;

int CONNECTION_connectToServer(char * ipAddress, int port);
void CONNECTION_programExecution(int socketFd, ConfigParams * configParams);

#endif
