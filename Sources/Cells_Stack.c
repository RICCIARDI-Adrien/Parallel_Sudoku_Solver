/** @file Cells_Stack.c
 * See Cells_Stack.h for description.
 * @author Adrien RICCIARDI
 */
#include <assert.h>
#include <Cells_Stack.h>
#include <Log.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Enable or disable this module debug messages. */
#define CELLS_STACK_IS_DEBUG_ENABLED 0

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
#if CELLS_STACK_IS_DEBUG_ENABLED
	/** Display a stack content.
	 * @param Pointer_Stack The stack to display content.
	 */
	static void CellsStackShow(TCellsStack *Pointer_Stack)
	{
		int i;
		
		LOG(CELLS_STACK_IS_DEBUG_ENABLED, "Count = %d\n", Pointer_Stack->Stack_Index);
		for (i = Pointer_Stack->Stack_Index - 1; i >= 0; i--) LOG(CELLS_STACK_IS_DEBUG_ENABLED, "Row = %d, Column = %d\n", Pointer_Stack->Cells_Coordinates[i].Row, Pointer_Stack->Cells_Coordinates[i].Column);
	}
#endif

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void CellsStackInitialize(TCellsStack *Pointer_Stack)
{
	Pointer_Stack->Stack_Index = 0;
}

int CellsStackPush(TCellsStack *Pointer_Stack, int Cell_Row, int Cell_Column)
{
	TCellsStackCellCoordinates *Pointer_Cell_Coordinates;
	
	// Make sure the stack is not overflowing
	assert(Pointer_Stack->Stack_Index < CELLS_STACK_ELEMENTS_COUNT);
	
	// Cache coordinates address
	Pointer_Cell_Coordinates = &Pointer_Stack->Cells_Coordinates[Pointer_Stack->Stack_Index];
	Pointer_Cell_Coordinates->Row = Cell_Row;
	Pointer_Cell_Coordinates->Column = Cell_Column;
	Pointer_Stack->Stack_Index++;
	
	#if CELLS_STACK_IS_DEBUG_ENABLED
		CellsStackShow(Pointer_Stack);
	#endif
	return 1;
}

int CellsStackReadTop(TCellsStack *Pointer_Stack, int *Pointer_Cell_Row, int *Pointer_Cell_Column)
{
	TCellsStackCellCoordinates *Pointer_Cell_Coordinates;
	
	// Can't use an assert here as this functions allows the caller to determine whether the stack is empty
	if (Pointer_Stack->Stack_Index <= 0) return 0;
	
	// Cache coordinates address
	Pointer_Cell_Coordinates = &Pointer_Stack->Cells_Coordinates[Pointer_Stack->Stack_Index - 1];
	*Pointer_Cell_Row = Pointer_Cell_Coordinates->Row;
	*Pointer_Cell_Column = Pointer_Cell_Coordinates->Column;
	
	#if CELLS_STACK_IS_DEBUG_ENABLED
		CellsStackShow(Pointer_Stack);
	#endif
	return 1;
}

void CellsStackRemoveTop(TCellsStack *Pointer_Stack)
{
	// Make sure stack is not empty to avoid underflowing it
	if (Pointer_Stack->Stack_Index > 0) Pointer_Stack->Stack_Index--;
}
