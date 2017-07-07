/* config.h - Zedit user configurable defines */

#define ZLIB		0	/* Requires zlib -lz */
#define SPELL		0	/* Requires libaspell */

#define UNDO        1
#define HUGE_FILES	1

/* If TERMCAP not defined defaults to ANSI */
/* #define TERMCAP */

/* Don't touch these unless you really know what you are doing. */
#define UNSIGNED_BYTES
#define HAVE_GLOBAL_MARKS
#define HAVE_BUFFER_MARKS
