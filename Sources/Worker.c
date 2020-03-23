/** @file Worker.c
 * See Worker.h for description.
 * @author Adrien RICCIARDI
 */
#include <assert.h>
#include <Configuration.h>
#include <errno.h>
#include <Grid.h>
#include <Log.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <Worker.h>

//-------------------------------------------------------------------------------------------------
// Private constants and macros
//-------------------------------------------------------------------------------------------------
/** Enable or disable this module debug messages. */
#define WORKER_IS_DEBUG_ENABLED 0

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Use a semaphore to count how many available workers remain. */
static sem_t Worker_Semaphore_Available_Workers_Count;

/** All workers data. */
static TWorker Workers[CONFIGURATION_WORKERS_MAXIMUM_COUNT];

/** The worker stack content. */
static TWorker *Pointer_Worker_Stack[CONFIGURATION_WORKERS_MAXIMUM_COUNT];
/** Worker stack index. Stack starts from 0 and grows. */
static volatile int Worker_Stack_Index = 0;
/** Allow to atomically access to the stack from worker threads. */
static pthread_mutex_t Worker_Stack_Mutex = PTHREAD_MUTEX_INITIALIZER;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Push a worker pointer to the workers stack top.
 * @param Pointer_Worker The pointer that will become the stack top.
 */
static void WorkerStackPush(TWorker *Pointer_Worker)
{
	pthread_mutex_lock(&Worker_Stack_Mutex);
	
	// Make sure the stack is not overflowing
	assert(Worker_Stack_Index < CONFIGURATION_WORKERS_MAXIMUM_COUNT);
	
	// Push value
	Pointer_Worker_Stack[Worker_Stack_Index] = Pointer_Worker;
	Worker_Stack_Index++;
	
	pthread_mutex_unlock(&Worker_Stack_Mutex);
}

/** Pop the top of the worker stack.
 * @return The worker that was on stack's top.
 */
static TWorker *WorkerStackPop(void)
{
	TWorker *Pointer_Worker;
	
	pthread_mutex_lock(&Worker_Stack_Mutex);
	
	// Make sure the stack is not underflowing
	assert(Worker_Stack_Index > 0);
	
	// Pop top value
	Worker_Stack_Index--;
	Pointer_Worker = Pointer_Worker_Stack[Worker_Stack_Index];
	
	pthread_mutex_unlock(&Worker_Stack_Mutex);
	
	return Pointer_Worker;
}

/** Solve a grid using the backtrack algorithm.
 * @return 0 if the grid could not be solved,
 * @return 1 if the grid was successfully solved.
 */
static int WorkerSolveGrid(TGrid *Pointer_Grid)
{
	int Row, Column;
	unsigned int Bitmask_Missing_Numbers, Tested_Number;
	
	// Find the first empty cell (don't remove the stack top now as the backtrack can return soon if no available number is found)
	if (CellsStackReadTop(&Pointer_Grid->Empty_Cells_Stack, &Row, &Column) == 0)
	{
		// No empty cell remain and there is no error in the grid : the solution has been found
		if (GridIsCorrectlyFilled(Pointer_Grid)) return 1;
		
		// A bad grid was generated...
		return 0;
	}
	
	// Get available numbers for this cell
	Bitmask_Missing_Numbers = GridGetCellMissingNumbers(Pointer_Grid, Row, Column);
	// If no number is available a bad grid has been generated... It's safe to return here as the top of the stack has not been altered
	if (Bitmask_Missing_Numbers == 0) return 0;
	
	// Try each available number
	for (Tested_Number = 0; Tested_Number < Pointer_Grid->Grid_Size; Tested_Number++)
	{
		// Loop until an available number is found
		if (!(Bitmask_Missing_Numbers & (1 << Tested_Number))) continue;
		
		// Try the number
		GridSetCellValue(Pointer_Grid, Row, Column, Tested_Number);
		GridRemoveCellMissingNumber(Pointer_Grid, Row, Column, Tested_Number);
		CellsStackRemoveTop(&Pointer_Grid->Empty_Cells_Stack); // Really try to fill this cell, removing it for next simulation step
		
		// Simulate next state
		if (WorkerSolveGrid(Pointer_Grid) == 1) return 1; // Good solution found, go to tree root
		
		// Bad solution found, restore old value
		GridSetCellValue(Pointer_Grid, Row, Column, GRID_EMPTY_CELL_VALUE);
		GridRestoreCellMissingNumber(Pointer_Grid, Row, Column, Tested_Number);
		CellsStackPush(&Pointer_Grid->Empty_Cells_Stack, Row, Column); // The cell is available again
	}
	// All numbers were tested unsuccessfully, go back into the tree
	return 0;
}

/** The function executed by the thread.
 * @param Pointer_Argument The worker owning this thread.
 * @return Unused value.
 */
static void *WorkerThreadFunction(void *Pointer_Argument)
{
	TWorker *Pointer_Worker = Pointer_Argument;
	
	// Retrieve TID
	Pointer_Worker->Thread_ID = syscall(SYS_gettid);
	
	// Add worker to "ready" stack
	WorkerStackPush(Pointer_Worker);
	
	// Threads do not gracefully terminate, they stop when program exits (this avoids checking for a lot of conditions or changing thread cancellation state)
	while (1)
	{
		// Wait for a grid to solve only if thread is not scheduled for termination (this could occur if thread is busy, trying to solve its grid, when it receives wait condition signal : thread misses wait condition signal and waits indefinitely)
		if (!Pointer_Worker->Is_Exit_Requested)
		{
			LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] Waiting for a grid to solve...\n", Pointer_Worker->Thread_ID);
			pthread_mutex_lock(&Pointer_Worker->Mutex_Wait_Condition);
			pthread_cond_wait(&Pointer_Worker->Wait_Condition, &Pointer_Worker->Mutex_Wait_Condition);
			pthread_mutex_unlock(&Pointer_Worker->Mutex_Wait_Condition);
		}
		
		// Should the thread terminate ?
		if (Pointer_Worker->Is_Exit_Requested)
		{
			LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] Worker exited as requested.\n", Pointer_Worker->Thread_ID);
			return NULL;
		}
		
		// Start solving
		LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] Starting solving grid.\n", Pointer_Worker->Thread_ID);
		Pointer_Worker->Is_Grid_Solved = WorkerSolveGrid(&Pointer_Worker->Grid);
		if (Pointer_Worker->Is_Grid_Solved) LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] A grid solution has been found.\n", Pointer_Worker->Thread_ID);
		else LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] Bad grid generated, worker is available for a new job.\n", Pointer_Worker->Thread_ID);
		
		// Tell that the worker is available for a new job
		WorkerStackPush(Pointer_Worker);
		sem_post(&Worker_Semaphore_Available_Workers_Count); // Increment the atomic counter
	}
	
	return NULL;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
int WorkerInitialize(int Maximum_Workers_Count)
{
	int i;
	pthread_t Thread_ID;
	
	// Create the atomic counter
	if (sem_init(&Worker_Semaphore_Available_Workers_Count, 0, Maximum_Workers_Count) != 0)
	{
		printf("[%s] Error : failed to create the workers semaphore.\n", __FUNCTION__);
		return -1;
	}
	
	// Create all workers
	for (i = 0; i < Maximum_Workers_Count; i++)
	{
		// Create wait condition mutex first because thread callback will use it
		if (pthread_mutex_init(&Workers[i].Mutex_Wait_Condition, NULL) != 0)
		{
			printf("[%s] Error : failed to create worker %d wait condition mutex (%s).\n", __FUNCTION__, i, strerror(errno));
			return -1;
		}
		
		// Create wait condition first because thread callback will use it
		if (pthread_cond_init(&Workers[i].Wait_Condition, NULL) != 0)
		{
			printf("[%s] Error : failed to create worker %d wait condition (%s).\n", __FUNCTION__, i, strerror(errno));
			return -1;
		}
		
		// Create thread
		Workers[i].Is_Exit_Requested = 0;
		if (pthread_create(&Thread_ID, NULL, WorkerThreadFunction, &Workers[i]) != 0) // Thread ID is not needed, so do not keep it
		{
			printf("[%s] Error : failed to create worker thread %d. (%s)\n", __FUNCTION__, i, strerror(errno));
			return -1;
		}
	}
	
	// Wait for all threads to become ready (each thread adds itself to workers stack when ready)
	while (Worker_Stack_Index < Maximum_Workers_Count);

	return 0;
}

void WorkerUninitialize(void)
{
	// Release the semaphore (threads are not gracefully released because they do not handle data that should be kept safe, like files)
	sem_destroy(&Worker_Semaphore_Available_Workers_Count);
}

void WorkerSolve(TWorker *Pointer_Worker)
{
	// No solution has been found yet
	Pointer_Worker->Is_Grid_Solved = 0;
	
	// Wake thread up
	pthread_mutex_lock(&Pointer_Worker->Mutex_Wait_Condition);
	pthread_cond_signal(&Pointer_Worker->Wait_Condition);
	pthread_mutex_unlock(&Pointer_Worker->Mutex_Wait_Condition);
}

int WorkerWaitForAvailableWorker(TWorker **Pointer_Pointer_Worker)
{
	TWorker *Pointer_Worker;
	int Available_Workers;
	
	// Block until a worker is available
	sem_wait(&Worker_Semaphore_Available_Workers_Count); // Decrement the atomic counter
	sem_getvalue(&Worker_Semaphore_Available_Workers_Count, &Available_Workers);
	
	// Retrieve first available worker
	Pointer_Worker = WorkerStackPop();
	*Pointer_Pointer_Worker = Pointer_Worker;
	LOG(WORKER_IS_DEBUG_ENABLED, "A worker with TID %d is available (remaining available workers = %d, workers stack index = %d).\n", Pointer_Worker->Thread_ID, Available_Workers, Worker_Stack_Index);
	
	// Did this worker solve the grid ?
	if (Pointer_Worker->Is_Grid_Solved) return 1;
	return 0;
}

void WorkerExit(TWorker *Pointer_Worker)
{
	// Tell thread to exit (no need to remove worker from stack as it has been already popped by WorkerWaitForAvailableWorker())
	LOG(WORKER_IS_DEBUG_ENABLED, "Telling thread with TID %d to exit.\n", Pointer_Worker->Thread_ID);
	Pointer_Worker->Is_Exit_Requested = 1;
	
	// Wake thread up
	pthread_mutex_lock(&Pointer_Worker->Mutex_Wait_Condition);
	pthread_cond_signal(&Pointer_Worker->Wait_Condition);
	pthread_mutex_unlock(&Pointer_Worker->Mutex_Wait_Condition);
}
