/** @file Configuration.h
 * Gather program configurable features.
 * @author Adrien RICCIARDI
 */
#ifndef H_CONFIGURATION_H
#define H_CONFIGURATION_H

//-------------------------------------------------------------------------------------------------
// Constants and macros
//-------------------------------------------------------------------------------------------------
// Disable debug if not explicitly enabled
#ifndef CONFIGURATION_IS_DEBUG_ENABLED
	/** Set to 1 to display debug messages, set to 0 to disable them. */
	#define CONFIGURATION_IS_DEBUG_ENABLED 0
#endif

/** Width and height of a grid in cells (a grid is made of CONFIGURATION_GRID_MAXIMUM_SIZE x CONFIGURATION_GRID_MAXIMUM_SIZE cells). */
#define CONFIGURATION_GRID_MAXIMUM_SIZE 16

/** The maximum amount of threads that allowed to run simultaneously. */
#define CONFIGURATION_THREADS_MAXIMUM_COUNT 128

#endif
