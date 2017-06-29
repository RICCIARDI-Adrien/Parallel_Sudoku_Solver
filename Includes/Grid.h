/** @file Grid.h
 * Gather multiple self-contained grids that can be used by several threads in parallel (each thread using only its own grid). Each grid provides all needed information for the solving algorithm.
 * @author Adrien RICCIARDI
 */
#ifndef H_GRID_H
#define H_GRID_H

#include <Configuration.h>

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** Specific value telling that the cell is empty (must be a value that can't be present in a grid to solve). */
#define GRID_EMPTY_CELL_VALUE 1000

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Load the grid content from a file.
 * @param Grid_ID In which grid to put the read content.
 * @param String_File_Name Name of the file describing the grid.
 * @return 0 if the grid was correctly loaded,
 * @return -1 if the file was not found,
 * @return -2 if the grid size is not 6, 9, 12 or 16,
 * @return -3 if cells data are bad.
 */
int GridLoadFromFile(unsigned int Grid_ID, char *String_File_Name);

/** Copy a grid cell values and internal bitmasks to another grid.
 * @param Source_Grid_ID The grid to copy from.
 * @param Destination_Grid_ID The grid to copy to.
 * @note Nothing is done if one or both provided IDs is bad.
 */
void GridCopy(int Source_Grid_ID, int Destination_Grid_ID); // TODO

/** Print the grid to the screen.
 * @param Grid_ID The grid to display.
 */
void GridShow(unsigned int Grid_ID);

#endif
