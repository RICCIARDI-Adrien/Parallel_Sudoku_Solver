/** @file Cells_Stack.h
 * A very simple implementation of an unique stack gathering all empty cells.
 * @author Adrien RICCIARDI
 */
#ifndef H_CELLS_STACK_H
#define H_CELLS_STACK_H

#include <Configuration.h>

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** How many elements (i.e. cell coordinates) a stack can hold. */
#define CELLS_STACK_ELEMENTS_COUNT (CONFIGURATION_GRID_MAXIMUM_SIZE * CONFIGURATION_GRID_MAXIMUM_SIZE)

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** A cell coordinates. */
typedef struct
{
	int Row; //!< The cell row coordinate.
	int Column; //!< The cell column coordinate.
} TCellsStackCellCoordinates;

/** A cell stack, made of a stack pointer and coordinates array. */
typedef struct
{
	TCellsStackCellCoordinates Cells_Coordinates[CELLS_STACK_ELEMENTS_COUNT]; //!< The stack content.
	int Stack_Pointer; //!< The stack top. This is a fully ascending stack.
} TCellsStack;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Set the stack as empty.
 * @param Pointer_Stack The stack to initialize.
 */
void CellsStackInitialize(TCellsStack *Pointer_Stack);

/** Push a cell coordinates on the top of the stack.
 * @param Pointer_Stack The stack to push to.
 * @param Cell_Row The cell row coordinate.
 * @param Cell_Column The cell column coordinate.
 * @return 1 if the cell coordinates were correctly pushed,
 * @return 0 if the stack is full.
 */
int CellsStackPush(TCellsStack *Pointer_Stack, int Cell_Row, int Cell_Column);

/** Get the top of the stack without removing it.
 * @param Pointer_Stack The stack to get top.
 * @param Pointer_Cell_Row On output, will contain the top of stack cell's row coordinate.
 * @param Pointer_Cell_Column On output, will contain the top of stack cell's column coordinate.
 * @return 1 if the cell was correctly poped,
 * @return 0 if the stack is empty.
 */
int CellsStackReadTop(TCellsStack *Pointer_Stack, int *Pointer_Cell_Row, int *Pointer_Cell_Column);

/** Remove the top of the stack. Do nothing if the stack is empty.
 * @param Pointer_Stack The stack to remove top.
 */
void CellsStackRemoveTop(TCellsStack *Pointer_Stack);

#endif
