#ifndef CONFIG_H
#define CONFIG_H

/* User configurable */

/* Define for dynamic buffer marks.
#define HAVE_BUFFER_MARKS
 */

/* Define for global buffer marks.
#define HAVE_GLOBAL_MARKS
 */

/* Define for huge file support.
#define HUGE_FILES
 */

/* Make huge file support threaded.
#define HUGE_THREADED
 */

/* Define for ZLIB support reading/writing files.
#define ZLIB
 */

/* Define for undo support.
#define UNDO
 */

/* Define for unsigned bytes. Default is char.
#define UNSIGNED_BYTES
 */

/* Define the page size. Default is currently 4k.
#define PGSIZE 4096
 */

/* Define for termcap rather than ansi. You want ansi.
#define TERMCAP
 */

/* You might need termio rather than termios on some old Unix systems.
#define HAVE_TERMIO
 */

/* OS specific settings */

#ifdef __QNXNTO__
#define __unix__
#endif

/* These should be set auto-magically. They are here for documentation
 * purposes.
 */

/* HAVE_ATOMIC */
/* NO_MEMCHR */
/* BUILTIN_REG */
/* ALLOW_NL (BUILTIN_REG ONLY) */
/* TBUFFERED */

#endif
