/** @file Main.c
 * Load the grid and divide the solving work between the available threads.
 * @author Adrien RICCIARDI
 */
#include <Configuration.h>
#include <Grid.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <Worker.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Main thread uses a specific grid. It is the last one to let the thread ID 0 use the grid ID 0 for more coherency. */
#define MAIN_THREAD_GRID_INDEX CONFIGURATION_WORKERS_MAXIMUM_COUNT

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
	unsigned int Row, Column, Bitmask_Missing_Numbers, Grid_Size, Tested_Number;
	int i, Have_All_Workers_Failed;
	
	// Cache grid size
	Grid_Size = Grids[MAIN_THREAD_GRID_INDEX].Grid_Size;
	
	// Walk across all cells to set the first empty one value, then provide it to a worker
	for (Row = 0; Row < Grid_Size; Row++)
	{
		for (Column = 0; Column < Grid_Size; Column++)
		{
			// Is this cell empty ?
			Bitmask_Missing_Numbers = GridGetCellMissingNumbers(&Grids[MAIN_THREAD_GRID_INDEX], Row, Column);
			if (Bitmask_Missing_Numbers == 0) continue; // The cell is not empty
			
			// Fill all available numbers
			for (Tested_Number = 0; Tested_Number < Grid_Size; Tested_Number++)
			{
				// Loop until an available number is found
				if (!(Bitmask_Missing_Numbers & (1 << Tested_Number))) continue;
				
				// Provide this grid to a worker
				GridSetCellValue(&Grids[MAIN_THREAD_GRID_INDEX], Row, Column, Tested_Number);
				
				// Find the first finished grid TODO optimize to avoid parsing all grids all the time
				WorkerWaitForAvailableWorker();
				
				for (i = 0; i < Main_Total_Allowed_Workers_Count; i++)
				{
					// Grid has been solved, stop searching
					if (Grids[i].State == GRID_STATE_SOLVING_SUCCESSED)
					{
						Pointer_Solved_Grid = &Grids[i]; // Cache the solved grid to avoid searching for it another time when the function terminates
						return 1;
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
			}
			
			// Restore empty cell, each worker must receive a grid with only one value altered
			GridSetCellValue(&Grids[MAIN_THREAD_GRID_INDEX], Row, Column, GRID_EMPTY_CELL_VALUE);
		}
	}
	
	// There is no more job to provide to workers, wait for a result
	do
	{
		// Search for a solved grid
		Have_All_Workers_Failed = 1;
		for (i = 0; i < Main_Total_Allowed_Workers_Count; i++)
		{
			// Grid has been solved, stop searching
			if (Grids[i].State == GRID_STATE_SOLVING_SUCCESSED)
			{
				Pointer_Solved_Grid = &Grids[i]; // Cache the solved grid to avoid searching for it another time when the function terminates
				return 1;
			}
			
			// Are some workers still searching ?
			if (Grids[i].State == GRID_STATE_BUSY) Have_All_Workers_Failed = 0;
		}
		
		// Avoid using 100% CPU
		usleep(1000000);
	} while (!Have_All_Workers_Failed);
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *String_Grid_File_Name;
	int Is_Grid_Solved, i;
	time_t Starting_Time, Ending_Time, Seconds, Minutes, Hours;
	
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
	if (Main_Total_Allowed_Workers_Count > CONFIGURATION_WORKERS_MAXIMUM_COUNT)
	{
		printf("Warning : program allows up to %d parallel threads, provided value %d has been decreased to %d.\n", CONFIGURATION_WORKERS_MAXIMUM_COUNT, Main_Total_Allowed_Workers_Count, CONFIGURATION_WORKERS_MAXIMUM_COUNT);
		Main_Total_Allowed_Workers_Count = CONFIGURATION_WORKERS_MAXIMUM_COUNT;
	}
	String_Grid_File_Name = argv[2];
	
	// Set all worker grids as available to use
	for (i = 0; i < Main_Total_Allowed_Workers_Count; i++) Grids[i].State = GRID_STATE_SOLVING_FAILED;
	
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
	
	// Display information about the grid to solve
	// Display file name
	printf("File : %s.\n", String_Grid_File_Name);
	// Display starting time
	Starting_Time = time(NULL);
	printf("Started solving on %s\n", ctime(&Starting_Time));
	// Display grid
	printf("Grid to solve :\n");
	GridShow(&Grids[MAIN_THREAD_GRID_INDEX]);
	putchar('\n');
	
	// Start solving
	Is_Grid_Solved = MainManageWorkers();
	
	// Show elapsed time
	Ending_Time = time(NULL);
	printf("Ended solving on %s", ctime(&Ending_Time));
	// Compute elapsed time
	printf("Elapsed time : ");
	Seconds = Ending_Time - Starting_Time;
	Minutes = Seconds / 60;
	Hours = Minutes / 60;
	Seconds -= Minutes * 60; // Adjust seconds, if less than 1 minute elapsed this subtracts 0
	Minutes -= Hours * 60; // Adjust minutes, if less than 1 hour elapsed this subtracts 0
	if (Hours > 0) printf("%ld hour(s) ", Hours);
	if ((Minutes > 0) || (Hours > 0)) printf("%ld minute(s) ", Minutes); // Always display minutes if hours are displayed
	printf("%ld second(s).\n\n", Seconds);
	
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
