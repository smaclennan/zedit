/* ansi.c - low level ansi terminal interface
 * Copyright (C) 1988-2013 Sean MacLennan
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

#include "keys.h"

struct key_array Tkeys[] = {
	{ "\033[A",	"up" },
	{ "\033[B",	"down" },
	{ "\033[C",	"right" },
	{ "\033[D",	"left" },

	{ "\033[2~",	"insert" },
	{ "\033[3~",	"delete" },
	{ "\033[5~",	"page up" },
	{ "\033[6~",	"page down" },
	{ "\033[7~",	"home" },
	{ "\033[8~",	"end" },

	{ "\033[11~",	"f1" },
	{ "\033[12~",	"f2" },
	{ "\033[13~",	"f3" },
	{ "\033[14~",	"f4" },
	{ "\033[15~",	"f5" },
	{ "\033[17~",	"f6" },
	{ "\033[18~",	"f7" },
	{ "\033[19~",	"f8" },
	{ "\033[20~",	"f9" },
	{ "\033[21~",	"f10" },
	{ "\033[23~",	"f11" },
	{ "\033[24~",	"f12" },

	{ "\033Oa",	"C-up" },
	{ "\033Ob",	"C-down" },
	{ "\033Oc",	"C-right" },
	{ "\033Od",	"C-left" },
	{ "\033[7^",	"C-home" },
	{ "\033[8^",	"C-end" },
};
#define N_KEYS ((int)(sizeof(Tkeys) / sizeof(struct key_array)))
