/** @file Worker.c
 * See Worker.h for description.
 * @author Adrien RICCIARDI
 */
#include <assert.h>
#include <Configuration.h>
#include <Grid.h>
#include <Log.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
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

/** All worker thread IDs. */
static pthread_t Worker_Thread_IDs[CONFIGURATION_WORKERS_MAXIMUM_COUNT];

/** Tell whether non-busy threads must exit. */
static int Worker_Is_Idle_Task_Stopped = 0;

/** The worker stack content. */
static TWorker *Pointer_Worker_Stack[CONFIGURATION_WORKERS_MAXIMUM_COUNT];
/** Worker stack index. Stack starts from 0 and grows. */
static int Worker_Stack_Index = 0;
/** Allow to atomically access to the stack from worker threads. */
static pthread_mutex_t Worker_Stack_Mutex = PTHREAD_MUTEX_INITIALIZER;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
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
 * @param Pointer_Grid_To_Solve The grid to solve.
 * @return Unused value.
 */
static void *WorkerThreadFunction(void *Pointer_Grid_To_Solve)
{
	int Is_Grid_Solved;
	TGrid *Pointer_Grid = Pointer_Grid_To_Solve;
	pid_t Thread_PID = 0;
	
	#if WORKER_IS_DEBUG_ENABLED
		Thread_PID = syscall(SYS_gettid);
	#endif
	
	// Threads do not gracefully terminate, they stop when program exits (this avoids checking for a lot of conditions or changing thread cancellation state)
	while (1)
	{
		LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] Waiting for a grid to solve...\n", Thread_PID);
		
		// Doing a busy loop consumes 100% CPU but allows the thread to start as soon as possible
		while (Pointer_Grid->State != GRID_STATE_BUSY)
		{
			if (Worker_Is_Idle_Task_Stopped)
			{
				LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] Idle worker exited.\n", Thread_PID);
				return NULL;
			}
		}
		
		// TODO fix this debug message, it is not thread-safe
		#if WORKER_IS_DEBUG_ENABLED
			LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] Starting solving grid :\n", Thread_PID);
			GridShow(Pointer_Grid);
			putchar('\n');
		#endif
		
		// Start solving
		Is_Grid_Solved = WorkerSolveGrid(Pointer_Grid);
		if (Is_Grid_Solved) Pointer_Grid->State = GRID_STATE_SOLVING_SUCCESSED;
		else
		{
			Pointer_Grid->State = GRID_STATE_SOLVING_FAILED;
			LOG(WORKER_IS_DEBUG_ENABLED, "[TID %d] Bad grid generated, worker is available for a new job.\n", Thread_PID);
		}
		
		// Tell that the worker is available for a new job
		sem_post(&Worker_Semaphore_Available_Workers_Count); // Increment the atomic counter
	}
	
	return NULL;
}

/** Push a worker pointer to the workers stack top.
 * Pointer_Worker The pointer that will become the stack top.
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

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
int WorkerInitialize(int Maximum_Workers_Count)
{
	int i;
	
	// Create the atomic counter
	if (sem_init(&Worker_Semaphore_Available_Workers_Count, 0, Maximum_Workers_Count) != 0)
	{
		printf("[%s] Error : failed to create the workers semaphore.\n", __FUNCTION__);
		return -1;
	}
	
	// Create all threads
	for (i = 0; i < Maximum_Workers_Count; i++)
	{
		if (pthread_create(&Worker_Thread_IDs[i], NULL, WorkerThreadFunction, &Grids[i]) != 0)
		{
			printf("[%s] Error : failed to create worker thread %d.\n", __FUNCTION__, i);
			return -1;
		}
	}

	return 0;
}

void WorkerUninitialize(void)
{
	// Release the semaphore (threads are not gracefully released because they do not handle data that should be kept safe, like files)
	sem_destroy(&Worker_Semaphore_Available_Workers_Count);
}

void WorkerSolve(TGrid *Pointer_Grid)
{
	Pointer_Grid->State = GRID_STATE_BUSY;
}

void WorkerWaitForAvailableWorker(void)
{
	sem_wait(&Worker_Semaphore_Available_Workers_Count); // Decrement the atomic counter
	LOG(WORKER_IS_DEBUG_ENABLED, "A worker is available.\n");
}

void WorkerStopIdleTasks(void)
{
	Worker_Is_Idle_Task_Stopped = 1;
}
