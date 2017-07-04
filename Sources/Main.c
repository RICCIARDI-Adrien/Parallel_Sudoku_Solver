/** @file Main.c
 * Load the grid and divide the solving work between the available threads.
 * @author Adrien RICCIARDI
 */
#include <Configuration.h>
#include <Grid.h>
#include <stdio.h>
#include <stdlib.h>
#include <Worker.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Main thread uses a specific grid. It is the last one to let the thread ID 0 use the grid ID 0 for more coherency. */
#define MAIN_THREAD_GRID_INDEX CONFIGURATION_THREADS_MAXIMUM_COUNT

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** How many threads to use to solve the sudoku. */
static int Main_Total_Allowed_Workers_Count;

/** Point to the solved grid when it has been found. */
static TGrid *Pointer_Solved_Grid;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Called when the program exits. */
static void MainExit(void)
{
	WorkerUninitialize();
}

/** Start solving a grid using the backtrack algorithm. Fill a number and transfer the grid to a worker.
 * @return 0 if the grid could not be solved,
 * @return 1 if the grid was successfully solved.
 */
static int MainManageWorkers(void)
{
	int Row, Column, i;
	static int Is_Grid_Solved = 0;
	unsigned int Bitmask_Missing_Numbers, Tested_Number;
	
	// Find the first empty cell (don't remove the stack top now as the backtrack can return soon if no available number is found) 
	if (CellsStackReadTop(&Grids[MAIN_THREAD_GRID_INDEX].Empty_Cells_Stack, &Row, &Column) == 0)
	{
		// No empty cell remain and there is no error in the grid : the solution has been found
		if (GridIsCorrectlyFilled(&Grids[MAIN_THREAD_GRID_INDEX])) return 1;
		
		// A bad grid was generated...
		#if CONFIGURATION_IS_DEBUG_ENABLED
			printf("[%s] Bad grid generated !\n", __FUNCTION__);
		#endif
		return 0;
	}
	
	// Get available numbers for this cell
	Bitmask_Missing_Numbers = GridGetCellMissingNumbers(&Grids[MAIN_THREAD_GRID_INDEX], Row, Column);
	// If no number is available a bad grid has been generated... It's safe to return here as the top of the stack has not been altered
	if (Bitmask_Missing_Numbers == 0) return 0;
	
	#if CONFIGURATION_IS_DEBUG_ENABLED
		printf("[%s] Available numbers for (row %d ; column %d) : ", __FUNCTION__, Row, Column);
		GridShowBitmask(Bitmask_Missing_Numbers);
	#endif
	
	// Try each available number
	for (Tested_Number = 0; Tested_Number < Grids[MAIN_THREAD_GRID_INDEX].Grid_Size; Tested_Number++)
	{
		// Loop until an available number is found
		if (!(Bitmask_Missing_Numbers & (1 << Tested_Number))) continue;
		
		// Try the number
		GridSetCellValue(&Grids[MAIN_THREAD_GRID_INDEX], Row, Column, Tested_Number);
		GridRemoveCellMissingNumber(&Grids[MAIN_THREAD_GRID_INDEX], Row, Column, Tested_Number);
		CellsStackRemoveTop(&Grids[MAIN_THREAD_GRID_INDEX].Empty_Cells_Stack); // Really try to fill this cell, removing it for next simulation step
		
		// Start a new worker with this specific grid
		WorkerWaitForAvailableWorker();

		// Find the first finished grid TODO optimize to avoid parsing all grids all the time
		for (i = 0; i < Main_Total_Allowed_Workers_Count; i++)
		{
			// Grid has been solved, stop searching
			if (Grids[i].State == GRID_STATE_SOLVING_SUCCESSED)
			{
				Pointer_Solved_Grid = &Grids[i]; // Cache the solved grid to avoid searching for it another time when the function terminates
				Is_Grid_Solved = 1;
				break;
			}
			
			// Is the grid available to start a new job ?
			if (Grids[i].State == GRID_STATE_SOLVING_FAILED)
			{
				// Fill the grid with the new one to solve
				GridCopy(&Grids[MAIN_THREAD_GRID_INDEX], &Grids[i]);
				WorkerSolve(&Grids[i]);
				break;
			}
		}

		// Simulate next state
		if (Is_Grid_Solved || (MainManageWorkers() == 1)) return 1; // Good solution found, go to tree root
		
		// Bad solution found, restore old value
		GridSetCellValue(&Grids[MAIN_THREAD_GRID_INDEX], Row, Column, GRID_EMPTY_CELL_VALUE);
		GridRestoreCellMissingNumber(&Grids[MAIN_THREAD_GRID_INDEX], Row, Column, Tested_Number);
		CellsStackPush(&Grids[MAIN_THREAD_GRID_INDEX].Empty_Cells_Stack, Row, Column); // The cell is available again
	}
	// All numbers were tested unsuccessfully, go back into the tree
	return 0;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *String_Grid_File_Name;
	int Is_Grid_Solved, i;
	
	// Show the title
	printf("+------------------------+\n");
	printf("| Parallel Sudoku Solver |\n");
	printf("+------------------------+\n\n");
	
	// Check parameters
	if (argc != 3)
	{
		printf("Usage : %s Maximum_Parallel_Threads Grid_File_Name\n", argv[0]);
		return EXIT_FAILURE;
	}
	Main_Total_Allowed_Workers_Count = atoi(argv[1]);
	if (Main_Total_Allowed_Workers_Count == 0)
	{
		printf("Error : maximum threads number must be a number greater than or equal to 1.\n");
		return EXIT_FAILURE;
	}
	String_Grid_File_Name = argv[2];
	
	// Set all worker grids as available to use
	for (i = 0; i < CONFIGURATION_THREADS_MAXIMUM_COUNT; i++) Grids[i].State = GRID_STATE_SOLVING_FAILED;
	
	// Tell how many workers can be started at the same time
	if (WorkerInitialize(Main_Total_Allowed_Workers_Count) != 0)
	{
		printf("Error : failed to initialize worker module.\n");
		return EXIT_FAILURE;
	}
	atexit(MainExit); // Automatically release the worker resources when the program exits
	
	// Try to load the grid file
	switch (GridLoadFromFile(&Grids[MAIN_THREAD_GRID_INDEX], String_Grid_File_Name))
	{
		case -1:
			printf("Error : can't open file %s.\n", String_Grid_File_Name);
			return EXIT_FAILURE;
			
		case -2:
			printf("Error : grid size or data is bad.\nThe maximum allowed size is %d.\n", CONFIGURATION_GRID_MAXIMUM_SIZE);
			return EXIT_FAILURE;
			
		case -3:
			printf("Error : bad grid file. There are not enough numbers to fill the grid.\n");
			return EXIT_FAILURE;
			
		default:
			break;
	}
	
	// Show file name
	printf("File : %s.\n\n", String_Grid_File_Name);
	// Show grid
	printf("Grid to solve :\n");
	GridShow(&Grids[MAIN_THREAD_GRID_INDEX]);
	putchar('\n');
	
	// Start solving
	Is_Grid_Solved = MainManageWorkers();
	
	// Show result
	if (Is_Grid_Solved)
	{
		printf("Solved grid :\n");
		GridShow(Pointer_Solved_Grid);
		putchar('\n');
		return EXIT_SUCCESS;
	}
	else
	{
		printf("Failed to solve this grid. Is it solvable ?\n");
		return EXIT_FAILURE;
	}
}
