/** @file Log.h
 * Provide a thread-safe logging system that can be enabled or disabled on a case-by-case basis.
 * @author Adrien RICCIARDI
 */
#ifndef H_LOG_H
#define H_LOG_H

//-------------------------------------------------------------------------------------------------
// Constants and macros
//-------------------------------------------------------------------------------------------------
#define LOG(Is_Enabled, Format_String, ...) do { if (Is_Enabled) LogPrintMessage("[%s:%d] " Format_String, __FUNCTION__, __LINE__, ##__VA_ARGS__); } while (0)

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Display a log message in thread-safe way.
 * @param Pointer_Format_String A printf-like format string.
 */
void LogPrintMessage(const char *Pointer_String_Format, ...);

#endif
