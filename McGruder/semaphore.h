#ifndef _MOD_SEMAPHORE_H_
#define _MOD_SEMAPHORE_H_

/////////////////////////////////////////////////////////////////////////////
/**
 *  @file   semaphore.h
 *  @author Jorge Solanas
 *  @date   October 5, 2011
 *  @brief  Library to facilitate the use of semaphores
 *
 *  (c) Copyright La Salle BCN, 2011.
 *  All rights reserved. Copying or other reproduction of this program except
 *  for archival purposes is prohibited without written consent of
 *  La Salle BCN.
 */
//////////////////////////////////////////////////////////////////////////////

//#define __USE_SVID
//#define __USE_XOPEN

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>
#include <stdlib.h>

/**
 * This file provides a simple but useful abstraction for
 * controlling acces by multiple processes to a common resource.
 */

/**
 * Union which must be explicitly declared by the application.
 * It is the fourth argument of the semctl function, and it is
 * optional, depending upon the operation requested.
 */
union semun
{
	int val;
	struct semid_ds * buf;
	unsigned short  * array;
};

/**
 * Struct used to indicate to a semaphore which operation is
 * going to be applied (number of the semaphore, operation and
 * flags).
 */
typedef struct
{
	unsigned short int sem_num;
	short int sem_op;
	short int sem_flg;
} sembuf;

/**
 * Struct with all the info about a semaphore. In this case,
 * only the id is specified.
 */
typedef struct
{
	int shmid;
} semaphore;

/**
 * Method which creates a semaphore
 * @param sem The var where semaphore will be created
 * @return int The result of the operation executed
 */
int SEM_constructor (semaphore * sem)
{
	assert (sem != NULL);
	sem->shmid = semget (IPC_PRIVATE, 1, IPC_CREAT | 0600);
	if (sem->shmid < 0) return sem->shmid;
    return 0;
}

/**
 * Method which initializes a semaphore
 * @param sem The semaphore to initialize
 * @param v The value to which the semaphores will be
 *          initialized
 * @return int The result of the operation executed
 */
int SEM_init (const semaphore * sem, const int v)
{
	unsigned short _v[1] = {v};
	assert (sem != NULL);
	return semctl (sem->shmid, 0, SETALL, _v);
}

/**
 * Method to destroy a semaphore
 * @param sem The semaphore to destroy
 * @return int The result of the operation executed
 */
int SEM_destructor (const semaphore * sem)
{
	assert (sem != NULL);
	return semctl (sem->shmid, 0, IPC_RMID, NULL);
}

/**
 * Method to apply a wait operation, in order to warn that a
 * process is going to use a common resource. The semaphore
 * counter will be decreased. If counter has the value zero, the
 * process will be added to the semaphore's queue
 * @param sem The semaphore where wait operation will be applied
 * @return int The result of the operation executed
 */
int SEM_wait (const semaphore * sem)
{
	struct sembuf o = {0, -1, SEM_UNDO};

	assert (sem != NULL);
	return semop(sem->shmid, &o, 1);
}

/**
 * Method to apply a signal operation, in order to warn that a
 * process has released a common resource. The semaphore counter
 * will be increased. If counter had the value zero and there
 * are processes in the queue, the top one has the access to the
 * common resource granted.
 * @param sem The semaphore where signal operation will be
 *            applied
 * @return int The result of the operation executed
 */
int SEM_signal (const semaphore * sem)
{
	struct sembuf o = {0, 1, SEM_UNDO};
	assert (sem != NULL);
	return semop(sem->shmid, &o, 1);
}

#endif /* _MOD_SEMAPHORE_H_ */
