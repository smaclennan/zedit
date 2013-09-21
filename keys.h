/* keys.h - Zedit key code defines
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

/*
	The keys defined by TERMCAP
	Don't change these without changing kbd.c
		- TCkey array
		- assumes numbers contiguous and start at TC_UP
	Max 32 keys (415)
*/
#define TC_UP			384
#define TC_DOWN			385
#define TC_RIGHT		386
#define TC_LEFT			387
#define TC_INSERT		388
#define TC_DELETE		389
#define TC_PPAGE		390
#define TC_NPAGE		391
#define TC_HOME			392
#define TC_END			393
#define TC_F1			394
#define TC_F2			395
#define TC_F3			396
#define TC_F4			397
#define TC_F5			398
#define TC_F6			399
#define TC_F7			400
#define TC_F8			401
#define TC_F9			402
#define TC_F10			403
#define TC_F11			404
#define TC_F12			405
#define TC_C_UP			406
#define TC_C_DOWN		407
#define TC_C_RIGHT		408
#define TC_C_LEFT		409
#define TC_C_HOME		410
#define TC_C_END		411
#define TC_BACK			412
#define NUMKEYS			413

struct key_array {
	char *key;
	char *label;
};

extern struct key_array Tkeys[];
extern unsigned int Key_mask;

#define K_NODEF			0xffff
#define SPECIAL_START		TC_UP
