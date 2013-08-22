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

/* OPERATING SYSTEM - We attempt to autodetect.
 * I have not tested anything other than Linux in a decade, you have
 * been warned!
 */
#ifdef __linux__
# define LINUX
# define SYSV4
#elif defined(__BSD__)
# define BSD
#elif defined(__unix__)
# define SYSV4
#else
# error OS not detected.
#endif

/* SCREEN DRIVER - define only one.
 * Unless you are running on an ancient dumb terminal, you probably want ANSI.
 * Linux wants ANSI, trust me on this ;)
 */
#define ANSI
/* #define TERMINFO */

/* USER CONFIGURABLE - don't define any, see if I care */
#define DBG		1		/* turn debugs on */
#define MINCONFIG	1		/* Minimal configuration */

#if !MINCONFIG
#define COMMENTBOLD	1		/* bold C comments */
#define UNDO            1		/* EXPERIMENTAL undo code */
#define HELP		1		/* Help */

#define SHELL		1		/* shell interface */
#define SPELL		1		/* ispell interface */
#define MAKE		1		/* make command */
#define TAGS		1		/* tag file support */
#endif

#include "configure.h"

/* DON'T TOUCH THESE */
#if SHELL || SPELL || TAGS || MAKE
#define PIPESH
#endif
#if defined(LINUX) && defined(ANSI)
/* Try defining this if you find the F6 and up keys are messed up. */
#define ANSI_KEY_HACK
#endif
