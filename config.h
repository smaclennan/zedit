/* config.h - Zedit user configurable defines
 *
 * This file should work for most modern Unix systems.
 * If not, try running ./configure
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define ANSI		1

/* Set SYSV4 if you need signal handlers to reset themselves. */
/* #define SYSV4 */

/* Define this if you have 16bit ints */
/* #define INT_IS_16BITS */

/* USER CONFIGURABLE - don't define any, see if I care */
#define SHELL		1
#define UNDO            0		/* EXPERIMENTAL undo code */

#endif
