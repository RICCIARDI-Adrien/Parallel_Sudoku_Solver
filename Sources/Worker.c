/** @file Worker.c
 * @see Worker.h for description.
 * @author Adrien RICCIARDI
 */
#include <Configuration.h>
#include <Grid.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <Worker.h>

//-------------------------------------------------------------------------------------------------
// Private macros
//-------------------------------------------------------------------------------------------------
// Ignore "misleading-indentation" warning that triggers when compiling in debug mode
#if CONFIGURATION_IS_DEBUG_ENABLED
	#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** How many workers are used. */
static int Worker_Maximum_Workers_Count;
/** Use a semaphore to count how many available workers remain. */
static sem_t Worker_Semaphore_Available_Workers_Count;

/** Tell that the program is exiting and that all threads must quit. */
static int Worker_Is_Program_Running = 1;

/** All worker thread IDs. */
static pthread_t Worker_Thread_IDs[CONFIGURATION_THREADS_MAXIMUM_COUNT];

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
		#if CONFIGURATION_IS_DEBUG_ENABLED
			printf("[%s] Bad grid generated !\n", __FUNCTION__);
		#endif
		return 0;
	}
	
	// Get available numbers for this cell
	Bitmask_Missing_Numbers = GridGetCellMissingNumbers(Pointer_Grid, Row, Column);
	// If no number is available a bad grid has been generated... It's safe to return here as the top of the stack has not been altered
	if (Bitmask_Missing_Numbers == 0) return 0;
	
	#if CONFIGURATION_IS_DEBUG_ENABLED
		printf("[%s] Available numbers for (row %d ; column %d) : ", __FUNCTION__, Row, Column);
		GridShowBitmask(Bitmask_Missing_Numbers);
	#endif
	
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
	#if CONFIGURATION_IS_DEBUG_ENABLED
		pid_t Thread_PID;
	#endif
		
	#if CONFIGURATION_IS_DEBUG_ENABLED
		Thread_PID = syscall(SYS_gettid);
	#endif
	
	while (Worker_Is_Program_Running)
	{
		#if CONFIGURATION_IS_DEBUG_ENABLED
			printf("[%s (TID %d)] Waiting for grid to solve...\n", __FUNCTION__, Thread_PID);
		#endif
		
		// TODO wait condition dans grid
		while (Pointer_Grid->State != GRID_STATE_BUSY);
		
		#if CONFIGURATION_IS_DEBUG_ENABLED
			printf("[%s (TID %d)] Grid to solve...\n", __FUNCTION__, Thread_PID);
			GridShow(Pointer_Grid);
			putchar('\n');
		#endif
		
		// Start solving
		Is_Grid_Solved = WorkerSolveGrid(Pointer_Grid);
		if (Is_Grid_Solved) Pointer_Grid->State = GRID_STATE_SOLVING_SUCCESSED;
		else Pointer_Grid->State = GRID_STATE_SOLVING_FAILED;
		
		// Tell that the worker is available for a new job
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
	
	// Create the atomic counter
	if (sem_init(&Worker_Semaphore_Available_Workers_Count, 0, Maximum_Workers_Count) != 0) return -1;
	Worker_Maximum_Workers_Count = Maximum_Workers_Count;
	
	// Create all threads
	for (i = 0; i < Worker_Maximum_Workers_Count; i++)
	{
		if (pthread_create(&Worker_Thread_IDs[i], NULL, WorkerThreadFunction, &Grids[i]) != 0) return -1;
	}

	return 0;
}

void WorkerUninitialize(void)
{
	//int i;
	
	// Stop all threads
	Worker_Is_Program_Running = 0;
	//for (i = 0; i < Worker_Semaphore_Available_Workers_Count; i++) pthread_join(Worker_Thread_IDs[i], NULL); // TODO send wait condition wake up and put solved grid to fasten threads quitting
	
	// Release the semaphore
	sem_destroy(&Worker_Semaphore_Available_Workers_Count);
}

void WorkerSolve(TGrid *Pointer_Grid)
{
	// TODO with wait condition
	Pointer_Grid->State = GRID_STATE_BUSY;
}

void WorkerWaitForAvailableWorker(void)
{
	sem_wait(&Worker_Semaphore_Available_Workers_Count); // Decrement the atomic counter
}
