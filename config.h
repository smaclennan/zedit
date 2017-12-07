/* config.h - Zedit user configurable defines */

#ifndef WIN32
#define ZLIB		0	/* Requires zlib -lz */
#define SPELL		0	/* Requires libaspell */
#endif

#define UNDO        1
#define HUGE_FILES	1

/* If TERMCAP not defined defaults to ANSI */
/* #define TERMCAP */

/* DOPIPES: Output in real time and you can continue editing.
 * default: Output in real time but you must wait until command complete.
 */
#ifdef __unix__
#define DOPIPES
#endif

/* Don't touch these unless you really know what you are doing. */
#define UNSIGNED_BYTES
#define HAVE_GLOBAL_MARKS
#define HAVE_BUFFER_MARKS
#ifdef WIN32
#define BUILTIN_REG
#endif
#ifdef __QNXNTO__
#define __unix__
#define NO_MEMRCHR
#endif
