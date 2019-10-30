/** @file Worker.h
 * Control a pool of worker threads that can solve different grids in parallel.
 * @author Adrien RICCIARDI
 */
#ifndef H_WORKER_H
#define H_WORKER_H

#include <Grid.h>

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

/** Start solving the provided grid. */
void WorkerSolve(TGrid *Pointer_Grid);

/** Block if no more worker is available. The function immediately returns if one or more workers are available to give them a grid to solve. */
void WorkerWaitForAvailableWorker(void);

#endif
