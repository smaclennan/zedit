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

/* OPERATING SYSTEM - define only one */
#define LINUX		1		/* Linux */
#define SYSV2		0		/* Unix System V Release 2/3 */
#define SYSV4		0		/* Unix System V Release 4   */
#define BSD		0		/* Berkely */
#define SUNBSD		0		/* Sun BSD */
#define ULTRIX		0		/* almost a BSD... */

/* SCREEN DRIVER */
#ifdef XWINDOWS
#define TERMINFO	0		/* don't change this one */
#define ANSI		0		/* don't change this one */
#else
#define XWINDOWS	0
#define TERMINFO	0		/* set terminfo here */
#define ANSI		1
#endif

/* USER CONFIGURABLE - don't define any, see if I care */
#define DBG		1		/* turn debugs on */
#define SLOW_DISK	0		/* File writes try to buffer up
					 * the data to a block size.
					 */
#define HAS_RESIZE	0		/* define this if have the
					 * resize command
					 */
#define XORCURSOR	1		/* Some machined (e.g. sun3 &&
					 * sun4) xor the attribute for
					 * the cursor. This means that
					 * when the cursor is on the
					 * mark, they both disappear.
					 * This fixes the problem.
					 */
#define UNDO            1		/* EXPERIMENTAL undo code */
#define COMMENTBOLD	1		/* bold C comments */

/* apps */
#define CALC		1		/* Calculator */
# define FLOATCALC	1		/* Allow floats in calc */
#define HELP		1		/* Help */
#define MAKE		1		/* make interface */
#define SHELL		1		/* shell interface */
#define SPELL		1		/* ispell interface */
#define TAGS		1		/* tag file support */

/* DON'T TOUCH THESE */
#if SUNBSD || ULTRIX
#undef  BSD
#define BSD		1
#endif
#if LINUX
#undef  SYSV4
#define SYSV4		1
#endif
#if SYSV4
#undef  SYSV2
#define SYSV2		1
#endif
#if HELP || MAKE || SHELL || SPELL || TAGS
#define PIPESH		1
#else
#define PIPESH		0
#endif
