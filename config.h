/* config.h - Zedit user configurable defines
 * Copyright (C) 1988-2010 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* OPERATING SYSTEM - We attempt to autodetect. */
#ifdef __linux__
# define LINUX
#elif defined(__BSD__)
# define BSD
/* # define SUNBSD */
/* # define ULTRIX */
#elif defined(__unix__)
# define SYSV4
/* # define SYSV2 */
#else
# error OS not detected.
#endif

/* SCREEN DRIVER - define only one. */
/* Unless you are running on an ancient dumb terminal, you probably want ANSI.
 * Especially Linux wants ANSI, trust me on this ;)
 */
#ifdef XWINDOWS
#define TERMINFO	0		/* don't change this one */
#define ANSI		0		/* don't change this one */
#else
#define TERMINFO	0		/* set terminfo here */
#define ANSI		1		/* set ANSI here */
#endif

/* USER CONFIGURABLE - don't define any, see if I care */
#define DBG		1		/* turn debugs on */
#define SLOW_DISK	0		/* File writes try to buffer up
					 * the data to a block size.
					 */
#define COMMENTBOLD	1		/* bold C comments */
#define FLOATCALC	1		/* Allow floats in calc */
#define UNDO            1		/* EXPERIMENTAL undo code */

#ifdef MINCONFIG
#undef DBG
#define DBG		0
#else
/* Warning: These are ifdefs, you must comment them out to disable them! */
#define CALC				/* Calculator */
#define HELP				/* Help */
#define SHELL				/* shell interface */
#define SPELL				/* ispell interface */
#define TAGS				/* tag file support */
#endif

/* DON'T TOUCH THESE */
#ifdef LINUX
#define SYSV4
#endif
#ifdef SYSV4
#define SYSV2
#endif
#if defined(HELP) || defined(SHELL) || defined(SPELL) || defined(TAGS) || defined(XWINDOWS)
#define PIPESH		1
#else
#define PIPESH		0
#endif
