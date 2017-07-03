/** @file Grid.h
 * Gather multiple self-contained grids that can be used by several threads in parallel (each thread using only its own grid). Each grid provides all needed information for the solving algorithm.
 * @author Adrien RICCIARDI
 */
#ifndef H_GRID_H
#define H_GRID_H

#include <Cells_Stack.h>
#include <Configuration.h>

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** Specific value telling that the cell is empty (must be a value that can't be present in a grid to solve). */
#define GRID_EMPTY_CELL_VALUE 1000

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** Contain all needed information to run the backtrack algorithm. */
typedef struct
{
	int Cells[CONFIGURATION_GRID_MAXIMUM_SIZE][CONFIGURATION_GRID_MAXIMUM_SIZE]; //!< Cells content.
	unsigned int Allowed_Numbers_Bitmask_Rows[CONFIGURATION_GRID_MAXIMUM_SIZE]; //!< Tell which numbers can be placed in each row (a bit is set when the number is allowed).
	unsigned int Allowed_Numbers_Bitmask_Columns[CONFIGURATION_GRID_MAXIMUM_SIZE]; //!< Tell which numbers can be placed in each column (a bit is set when the number is allowed).
	unsigned int Allowed_Numbers_Bitmask_Squares[CONFIGURATION_GRID_MAXIMUM_SIZE]; //!< Tell which numbers can be placed in each square (a bit is set when the number is allowed).
	TCellsStack Empty_Cells_Stack; //!< Contain all grid empty cells to avoid loosing time searching for them.
} TGrid;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Load the grid content from a file.
 * @param Pointer_Grid In which grid to put the read content.
 * @param String_File_Name Name of the file describing the grid.
 * @return 0 if the grid was correctly loaded,
 * @return -1 if the file was not found,
 * @return -2 if the grid size is not 6, 9, 12 or 16,
 * @return -3 if cells data are bad.
 */
int GridLoadFromFile(TGrid *Pointer_Grid, char *String_File_Name);

/** Copy a grid cell values and internal bitmasks to another grid.
 * @param Pointer_Grid_Source The grid to copy from.
 * @param Pointer_Grid_Destination The grid to copy to.
 */
void GridCopy(TGrid *Pointer_Grid_Source, TGrid *Pointer_Grid_Destination); // TODO

/** Print the grid to the screen.
 * @param Pointer_Grid The grid to display.
 */
void GridShow(TGrid *Pointer_Grid);

#endif
