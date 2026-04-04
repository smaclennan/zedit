#ifndef CONFIG_H
#define CONFIG_H

/* User configurable */

/* Define for dynamic buffer marks.
#define BUFFER_MARKS 1
 */

/* Define for global buffer marks.
#define GLOBAL_MARKS 1
 */

/* Define for huge file support.
#define HUGE_FILES 1
 */

/* Make huge file support threaded.
#define HUGE_THREADED 1
 */

/* Define for ZLIB support reading/writing files.
#define ZLIB 1
 */

/* Define for undo support.
#define UNDO 1
 */

/* Define for unsigned bytes. Default is char.
#define UNSIGNED_BYTES 1
 */

/* Define the page size. Default is currently 4k.
#define PGSIZE 4096
 */

/* Define for termcap rather than ansi. You want ansi.
#define TERMCAP 1
 */

/* You might need termio rather than termios on some old Unix systems.
#define HAVE_TERMIO 1
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
