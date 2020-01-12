/** @file Configuration.h
 * Gather program configurable features.
 * @author Adrien RICCIARDI
 */
#ifndef H_CONFIGURATION_H
#define H_CONFIGURATION_H

//-------------------------------------------------------------------------------------------------
// Constants and macros
//-------------------------------------------------------------------------------------------------
/** Width and height of a grid in cells (a grid is made of CONFIGURATION_GRID_MAXIMUM_SIZE x CONFIGURATION_GRID_MAXIMUM_SIZE cells). */
#define CONFIGURATION_GRID_MAXIMUM_SIZE 16

/** The maximum amount of threads that are allowed to run simultaneously. */
#define CONFIGURATION_WORKERS_MAXIMUM_COUNT 1024

#endif
