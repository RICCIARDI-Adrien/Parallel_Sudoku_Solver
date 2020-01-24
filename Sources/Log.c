/** @file Log.c
 * See Log.h for description.
 * @author Adrien RICCIARDI
 */
#include <Log.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
static pthread_mutex_t Logs_Mutex = PTHREAD_MUTEX_INITIALIZER;

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void LogPrintMessage(const char *Pointer_String_Format, ...)
{
	va_list Arguments_List;
	
	pthread_mutex_lock(&Logs_Mutex);
	
	// Display log message
	va_start(Arguments_List, Pointer_String_Format);
	vprintf(Pointer_String_Format, Arguments_List);
	va_end(Arguments_List);
	
	pthread_mutex_unlock(&Logs_Mutex);
}
