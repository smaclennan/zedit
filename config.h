/* config.h - Zedit user configurable defines */

#if defined(DOS)
#define INT_IS_16BITS	1
#elif defined(WIN32)
#define SHELL		1
#else
#define HAVE_TERMIOS	1
#define ZLIB		1	/* Requires zlib -lz */
#define SHELL		1
#define SPELL		1	/* Requires libaspell */
#define UNDO		0	/* EXPERIMENTAL */
#endif

/* This overrides the SHELL version */
#define INTERNAL_GREP	1
