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
/** Start a new worker thread with the provided grid.
 * @return 0 if the worker successfully started,
 * @return -1 if an error happened.
 */
int WorkerStart(TGrid *Pointer_Grid);

#endif