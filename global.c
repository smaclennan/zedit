/* global.c - all globals that are saved to file
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

#include "z.h"

char Lbufname[BUFNAMMAX + 1] = MAINBUFF;	/* Zswitchto */
char Fname[PATHMAX + 1] = "";			/* Zfindfile, Zfileread */

char Command[STRMAX + 1] = "";			/* Zcmd, Zcmdtobuff */
char mkcmd[STRMAX + 1] = "make";		/* Zmake, Vars */
char grepcmd[STRMAX + 1] = "grep -n";		/* Zgrep, Vars */

char old[STRMAX + 1] = "";			/* Search string */
char new[STRMAX + 1] = "";			/* Replace string */
Boolean searchdir[3] = {0, 0};			/* Current direction for Again.
						 * searchdir[2] unused - padding
						 * for hp.
						 */

char Calc_str[STRMAX + 1] = "";			/* Zcalc */
