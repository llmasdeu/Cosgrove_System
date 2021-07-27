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
#include "Paquita.h"
#include "Paquita_List.h"

/**
 * Funció encarregada d'inicialitzar la cua de missatges.
 * @param  keyPaquita Clau de la cua de missatges de Paquita.
 * @param  idPaquita  Identificador de la cua de missatges de Paquita.
 * @return            0 si no hi ha hagut cap problema. -1 si hi ha hagut problemes.
 */
int PAQUITA_initializeMessagesQueue(key_t * keyPaquita, int * idPaquita) {
  *keyPaquita = ftok("Paquita.c", 0);

  // Comprovem si hi ha hagut algun problema en la inicialització.
  if (*keyPaquita == (key_t) -1) {
    return -1;
  }

  // Obtenim l'identificador de la cua de missatges.
  *idPaquita = msgget(*keyPaquita, 0600 | IPC_CREAT);

  // Comprovem si ha sigut satisfactori.
  if (*idPaquita < 0) {
    msgctl(*idPaquita, IPC_RMID, (struct msqid_ds *) NULL);

    return -1;
  }

  return 0;
}

/**
 * Funció encarregada de reiniciar les estadístiques.
 * @param statistics Estadístiques a reiniciar.
 */
void PAQUITA_resetStatistics(StatisticsPaquita * statistics) {
  statistics->imageFiles = 0;
  statistics->textFiles = 0;
  statistics->totalSize = 0.0;
  statistics->averageConstellations = 0.0;
  statistics->constellationsNumber = 0;
  statistics->averageDensity = 0.0;
  statistics->maxMagnitude = 0.0;
  statistics->minMagnitude = 0.0;
}

/**
 * Funció encarregada de llegir la informació dels fitxers.
 * @param mPaquita   Missatge llegit.
 * @param statistics Estadístiques de Paquita.
 */
void PAQUITA_filesQueueExecution(MessagePaquita mPaquita, StatisticsPaquita * statistics) {
  double kb;

  kb = (double) mPaquita.fMessage.fileSize / 1000;
  statistics->totalSize += kb;

  if (mPaquita.fMessage.fileType == IMAGE_FILE_TYPE_QUEUE) {
    statistics->imageFiles++;
  } else if (mPaquita.fMessage.fileType == TEXT_FILE_TYPE_QUEUE) {
    statistics->textFiles++;
  }
}

/**
 * Funció encarregada de desar les dades de les constel·lacions.
 * @param cList    Llista de les constel·lacions.
 * @param cMessage Dades de la constel·lació.
 */
void PAQUITA_storeConstellationData(List * cList, ConstellationsMessagePaquita cMessage) {
  Constellation * constellation = (Constellation*) malloc(sizeof(Constellation));
  constellation->code = (char*) malloc((strlen(cMessage.code) + 1) * sizeof(char));

  // Copiem les dades de la constel·lació.
  strcpy(constellation->code, cMessage.code);
  constellation->density = cMessage.density;
  constellation->magnitude = cMessage.magnitude;

  // Busquem la constel·lació en el registre.
  if (PAQUITA_LIST_search(cList, cMessage.code) == 0) {
    PAQUITA_LIST_add(cList, *constellation);
  }

  // Alliberem la memòria demanada.
  free(constellation);
  constellation = NULL;
}

/**
 * Funció encarregada de computar les estadístiques de les constel·lacions.
 * @param cList      Llista de les constel·lacions.
 * @param statistics Estadístiques de Paquita.
 */
void PAQUITA_computeConstellationData(List * cList, StatisticsPaquita * statistics) {
  int times = 0, totalConstellations = 0;
  float density = 0.0;
  Constellation * constellation;

  // Anem a l'inici de la llista.
  PAQUITA_LIST_beginning(cList);

  while (!PAQUITA_LIST_end(*cList)) {
    constellation = (Constellation*) malloc(sizeof(Constellation));

    PAQUITA_LIST_view(*cList, constellation);
    PAQUITA_LIST_forward(cList);

    totalConstellations++;
    times += constellation->times;
    density += constellation->density;

    // Alliberem la memòria demanada.
    free(constellation);
    constellation = NULL;
  }

  statistics->averageConstellations = totalConstellations / statistics->textFiles;
  statistics->constellationsNumber = totalConstellations;
  statistics->averageDensity = density / times;
  statistics->minMagnitude = cList->minMagnitude;
  statistics->maxMagnitude = cList->maxMagnitude;
}

/**
 * Funció encarregada de llegir la informació de les constel·lacions.
 * @param cList      Llista de les constel·lacions.
 * @param mPaquita   Missatge llegit.
 * @param statistics Estadístiques de Paquita.
 */
void PAQUITA_constellationsQueueExecution(List * cList, MessagePaquita * mPaquita, StatisticsPaquita * statistics) {
  // Desem el contingut a la llista, i calculem els paràmetres.
  PAQUITA_storeConstellationData(cList, mPaquita->cMessage);
  PAQUITA_computeConstellationData(cList, statistics);
}

/**
 * Funció encarregada de definir la informació de la resposta.
 * @param statistics Estadístiques de Paquita.
 * @param mPaquita   Missatge de resposta.
 */
void PAQUITA_setStatisticsData(StatisticsPaquita statistics, MessagePaquita * mPaquita) {
  mPaquita->queueType = FEEDBACK_TYPE_QUEUE;
  mPaquita->killPaquita = 0;
  mPaquita->statistics.imageFiles = statistics.imageFiles;
  mPaquita->statistics.textFiles = statistics.textFiles;
  mPaquita->statistics.totalSize = statistics.totalSize;
  mPaquita->statistics.averageConstellations = statistics.averageConstellations;
  mPaquita->statistics.constellationsNumber = statistics.constellationsNumber;
  mPaquita->statistics.averageDensity = statistics.averageDensity;
  mPaquita->statistics.maxMagnitude = statistics.maxMagnitude;
  mPaquita->statistics.minMagnitude = statistics.minMagnitude;
}

/**
 * Funció encarregada d'escoltar amb la cua de missatges.
 * @param idPaquita      Identificador de la cua de missatges de Paquita.
 * @param paquitaRunning Flag indicatiu de la execució de Paquita
 * @param statistics     Estadístiques de Paquita.
 */
void paquitaExecution(int idPaquita, int * paquitaRunning, StatisticsPaquita * statistics) {
  int n = 0;
  List cList;
  MessagePaquita * mPaquita = (MessagePaquita*) malloc(sizeof(MessagePaquita));

  // Creem la llista de les constel·lacions.
  PAQUITA_LIST_create(&cList);

  while (*paquitaRunning == 1) {
    n = msgrcv(idPaquita, mPaquita, sizeof(MessagePaquita), 0, 0);

    if (n > 0) {
      if (mPaquita->killPaquita == 1) {
        *paquitaRunning = 0;
      } else {
        if (mPaquita->queueType == FILES_TYPE_QUEUE) {
          PAQUITA_filesQueueExecution(*mPaquita, statistics);
        } else if (mPaquita->queueType == CONSTELLATIONS_TYPE_QUEUE) {
          PAQUITA_constellationsQueueExecution(&cList, mPaquita, statistics);
        }

        // Generem el missatge de resposta, i l'enviem.
        PAQUITA_setStatisticsData(*statistics, mPaquita);
        msgsnd(idPaquita, mPaquita, sizeof(MessagePaquita), IPC_NOWAIT);
      }
    }
  }

  // Destruim la llista de les constel·lacions.
  PAQUITA_LIST_destroy(&cList);

  // Alliberem la memòria demanada
  free(mPaquita);
  mPaquita = NULL;
}

/**
 * Funció encarregada de destruïr la cua de missatges.
 * @param idPaquita [description]
 */
void PAQUITA_destroyMessagesQueue(int idPaquita) {
  msgctl(idPaquita, IPC_RMID, (struct msqid_ds *) NULL);
}

/**
 * Funció princial de Paquita.
 */
void PAQUITA_mainPaquita() {
  int idPaquita, paquitaRunning = 1;
  key_t keyPaquita;
  StatisticsPaquita statistics;

  // Intentem inicialitzar la cua de missatges.
  if (PAQUITA_initializeMessagesQueue(&keyPaquita, &idPaquita) == -1) {
    write(1, ERROR_CREATING_QUEUES, strlen(ERROR_CREATING_QUEUES));
  } else {
    // Reiniciem les estadístiques.
    PAQUITA_resetStatistics(&statistics);

    // Escoltem la cua de missatges...
    paquitaExecution(idPaquita, &paquitaRunning, &statistics);

    // Destruïm la cua de missatges.
    PAQUITA_destroyMessagesQueue(idPaquita);
  }
}
