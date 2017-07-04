/** @file Worker.c
 * @see Worker.h for description.
 * @author Adrien RICCIARDI
 */
#include <Configuration.h>
#include <Grid.h>
#include <Worker.h>

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
		//if (GridIsCorrectlyFilled()) return 1; TODO
		return 1; // TEST
		
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
		
		#if CONFIGURATION_IS_DEBUG_ENABLED
			printf("[%s] Modified grid :\n", __FUNCTION__);
			GridShowDifferences(GRID_COLOR_CODE_BLUE);
			putchar('\n');
		#endif

		// Simulate next state
		if (WorkerSolveGrid(Pointer_Grid) == 1) return 1; // Good solution found, go to tree root
		
		// Bad solution found, restore old value
		GridSetCellValue(Pointer_Grid, Row, Column, GRID_EMPTY_CELL_VALUE);
		GridRestoreCellMissingNumber(Pointer_Grid, Row, Column, Tested_Number);
		CellsStackPush(&Pointer_Grid->Empty_Cells_Stack, Row, Column); // The cell is available again
		
		#if CONFIGURATION_IS_DEBUG_ENABLED
			printf("[%s] Restored grid :\n", __FUNCTION__);
			GridShowDifferences(GRID_COLOR_CODE_RED);
			putchar('\n');
		#endif
	}
	// All numbers were tested unsuccessfully, go back into the tree
	return 0;
}

#if 0
/** The function executed by the thread.
 * @param Pointer_Grid The grid to solve.
 * @return 0 (casted to void *) if the grid could not be solved,
 * @return 1 (casted to void *) if the grid was solved.
 */
static void *WorkerThreadFunction(void *Pointer_Grid)
{
	long Is_Grid_Solved; // Use an integer type that reflects the machine native word size, so it is always of the same size than memory address, thus than pointer
	
	Is_Grid_Solved = WorkerSolveGrid(Pointer_Grid);
	return (void *) Is_Grid_Solved;
}
#endif

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
int WorkerStart(TGrid *Pointer_Grid)
{
	// TEST
	return WorkerSolveGrid(Pointer_Grid);
}
