/* config.h - Zedit user configurable defines */

#ifndef WIN32
#define ZLIB		0	/* Requires zlib -lz */
#define SPELL		0	/* Requires libaspell */
#endif
#ifdef __linux__
#define GPM_MOUSE	0	/* Requires libgpm */
#endif

#define SHOW_REGION	1
#define UNDO        1

/* If nothing defined defaults to ANSI */
/* #define TERMCAP */
/* #define TERMCAP_KEYS */

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
#define HAVE_FREEMARK
#ifdef WIN32
#define BUILTIN_REG
#endif
