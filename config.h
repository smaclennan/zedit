/* config.h - Zedit user configurable defines */

#define CONFIG_H

#define ZLIB			0	/* Requires zlib -lz */
#define SPELL			0	/* Requires libaspell */
#define UNDO			1
#define HUGE_FILES		0
#define HUGE_THREADED		0

/* If TERMCAP not defined defaults to ANSI */
/* #define TERMCAP */
/* If TERMINFO not defined defaults to ANSI */
/* #define TERMINFO */

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
#define WANT_FLOATS
#ifdef __QNXNTO__
#undef  __unix__
#define __unix__
#endif
