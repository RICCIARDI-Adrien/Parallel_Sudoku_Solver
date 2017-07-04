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
	unsigned int Grid_Size; //!< Grid size (height and width) in cells. It is not supposed to change during program execution but it is made available for functions that do not have access to grid module private variables.
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

/** Retrieve the allowed numbers for a cell.
 * @param Pointer_Grid The concerned grid.
 * @param Cell_Row Row coordinate.
 * @param Cell_Column Column coordinate.
 * @return A bitmask containing all allowed numbers.
 */
unsigned int GridGetCellMissingNumbers(TGrid *Pointer_Grid, unsigned int Cell_Row, unsigned int Cell_Column);

/** Set the value of a cell.
 * @param Pointer_Grid The grid to set cell value.
 * @param Cell_Row Row coordinate of the cell.
 * @param Cell_Column Column coordinate of the cell.
 * @param Cell_Value Value of the cell.
 * @warning No check is done on the provided coordinates, be sure to not cross grid bounds.
 */
void GridSetCellValue(TGrid *Pointer_Grid, unsigned int Cell_Row, unsigned int Cell_Column, int Cell_Value);

/** Remove a number from the cell missing ones lists.
 * @param Pointer_Grid The concerned grid.
 * @param Cell_Row Row coordinate of the cell.
 * @param Cell_Column Column coordinate of the cell.
 * @param Number The number to remove from the cell missing numbers.
 */
void GridRemoveCellMissingNumber(TGrid *Pointer_Grid, unsigned int Cell_Row, unsigned int Cell_Column, int Number);

/** Restore a number into the cell missing ones lists.
 * @param Pointer_Grid The concerned grid.
 * @param Cell_Row Row coordinate of the cell.
 * @param Cell_Column Column coordinate of the cell.
 * @param Number The number to restore to the cell missing numbers.
 */
void GridRestoreCellMissingNumber(TGrid *Pointer_Grid, unsigned int Cell_Row, unsigned int Cell_Column, int Number);

#endif
