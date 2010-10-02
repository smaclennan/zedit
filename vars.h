/* vars.h - Zedit variable defines
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

/* The variable structure. */
struct avar {
	char *vname;
	int   vtype;
	union {
		unsigned val;
		char *str;
	} u;
};
extern struct avar Vars[];

/* Handy macros */
#define VAR(n)		Vars[n].u.val
#define VARSTR(n)	Vars[n].u.str

/* var defines */
#define VADDNL		0
#define VASCHAR		(VADDNL + 1)
#define VASEXTS		(VASCHAR + 1)
#define VBACKUP		(VASEXTS + 1)
#define VCEXTS		(VBACKUP + 1)
#define VCLEAR		(VCEXTS + 1)
#define VCTABS		(VCLEAR + 1)
#define VDATESTR	(VCTABS + 1)
#define VDOSAVE		(VDATESTR + 1)
#define VEXACT		(VDOSAVE + 1)
#define VEXECONMATCH	(VEXACT + 1)
#define VEXPAND		(VEXECONMATCH + 1)
#define VFILLWIDTH	(VEXPAND + 1)
#define VFLOW		(VFILLWIDTH + 1)
#define VFONT		(VFLOW + 1)
#define VGREP		(VFONT + 1)
#define VKILLLINE	(VGREP + 1)
#define VLINES		(VKILLLINE + 1)
#define VMAIL		(VLINES + 1)
#define VMAKE		(VMAIL + 1)
#define VMARGIN		(VMAKE + 1)
#define VMATCH		(VMARGIN + 1)
#define VNORMAL		(VMATCH + 1)
#define VOVWRT		(VNORMAL + 1)
#define VPRINT		(VOVWRT + 1)
#define VSAVE		(VPRINT + 1)
#define VSHOWCWD	(VSAVE + 1)
#define VSILENT		(VSHOWCWD + 1)
#define VSINGLE		(VSILENT + 1)
#define VSPACETAB	(VSINGLE + 1)
#define VTABS		(VSPACETAB + 1)
#define VTAG		(VTABS + 1)
#define VSEXTS		(VTAG + 1)
#define VTEXTS		(VSEXTS + 1)
#define VVISBELL	(VTEXTS + 1)
#define VARNUM		(VVISBELL + 1)


/* the variable types */
#define STRING		0
#define DECIMAL		1
#define FLAG		2
#define NUMERRSTR	2
