/** @file Worker.h
 * Control a pool of worker threads that can solve different grids in parallel.
 * @author Adrien RICCIARDI
 */
#ifndef H_WORKER_H
#define H_WORKER_H

#include <Configuration.h> // TEST
#include <Grid.h>
#include <pthread.h>

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** A worker is a thread that tries to solve the grid provided to it. */
typedef struct
{
	TGrid Grid; //!< The grid the worker must solve.
	pthread_cond_t Wait_Condition; //!< Idle the worker thread until a job is received.
	pthread_mutex_t Mutex_Wait_Condition; //!< The mutex granting atomic access to the wait condition.
} TWorker;

// TEST
extern TWorker Workers[CONFIGURATION_WORKERS_MAXIMUM_COUNT];

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Tell the worker subsystem how many threads can be started simultaneously.
 * @param Maximum_Workers_Count How many threads can be started at the same time.
 * @return 0 on success,
 * @return -1 if an error occurred.
 * @note This function prints an error message if an error occurs.
 */
int WorkerInitialize(int Maximum_Workers_Count);

/** Release worker resources. */
void WorkerUninitialize(void);

/** Tell the specified worker to start solving its grid.
 * @param Pointer_Worker The worker that must start its job.
 */
void WorkerSolve(TWorker *Pointer_Worker);

/** Block if no more worker is available. The function immediately returns if one or more workers are available to give them a grid to solve. */
void WorkerWaitForAvailableWorker(void);

/** Tell workers that no more tasks will be started, so unemployed workers can retire. */
void WorkerStopIdleTasks(void);

#endif
