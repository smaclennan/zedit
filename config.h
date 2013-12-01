/* config.h - Zedit user configurable defines
 *
 * This file should work for most modern Unix systems.
 * If not, try running ./configure
 */

/* USER CONFIGURABLE - don't define any, see if I care */
#ifndef WIN32
#define SHELL		1
#define SPELL		1		/* Requires libaspell */
#define ZLIB		1		/* Requires zlib */
#define TAGS		1
#endif

#define UNDO            1		/* EXPERIMENTAL undo code */
