/* config.h - Zedit user configurable defines
 *
 * This file should work for most modern Unix systems.
 * If not, try running ./configure
 */

#ifdef WIN32
#define NO_POLL 1
#endif

/* USER CONFIGURABLE - don't define any, see if I care */
#define SHELL		0
#define SPELL		0		/* Requires libaspell */
#define ZLIB		0		/* Requires zlib */
#define UNDO            0		/* EXPERIMENTAL undo code */
