/** @file Main.c
 * Load the grid and divide the solving work between the available threads.
 * @author Adrien RICCIARDI
 */
#include <Configuration.h>
#include <Grid.h>
#include <stdio.h>
#include <stdlib.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Main thread uses a specific grid. It is the last one to let the thread ID 0 use the grid ID 0 for more coherency. */
#define MAIN_THREAD_GRID_INDEX CONFIGURATION_THREADS_MAXIMUM_COUNT

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** How many threads to use to solve the sudoku. */
//static unsigned char Main_Maximum_Allowed_Threads_Count;

/** All worker grids (one per worker, plus one for the main thread). */
static TGrid Main_Grids[CONFIGURATION_THREADS_MAXIMUM_COUNT + 1];

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char *String_Grid_File_Name;
	
	// Show the title
	printf("+------------------------+\n");
	printf("| Parallel Sudoku Solver |\n");
	printf("+------------------------+\n\n");
	
	// Check parameters
	if (argc != 2) // TODO Main_Maximum_Allowed_Threads_Count
	{
		printf("Usage : %s Grid_File_Name\n", argv[0]);
		return EXIT_FAILURE;
	}
	String_Grid_File_Name = argv[1];
	
	// Try to load the grid file
	switch (GridLoadFromFile(&Main_Grids[MAIN_THREAD_GRID_INDEX], String_Grid_File_Name))
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
	GridShow(&Main_Grids[MAIN_THREAD_GRID_INDEX]);
	putchar('\n');
	
	
}