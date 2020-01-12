/** @file Log.c
 * See Log.hpp for description.
 * @author Adrien RICCIARDI
 */
#include <Log.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void LogPrintMessage(const char *Pointer_String_Format, ...)
{
	va_list Arguments_List;
	
	va_start(Arguments_List, Pointer_String_Format);
	vprintf(Pointer_String_Format, Arguments_List);
	va_end(Arguments_List);
}
