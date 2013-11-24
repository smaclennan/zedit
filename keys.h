/* keys.h - Zedit key code defines
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

#ifndef _KEYS_H_
#define _KEYS_H_

#define SPECIAL_START		384
#define TC_UP			(SPECIAL_START + 0)
#define TC_DOWN			(SPECIAL_START + 1)
#define TC_RIGHT		(SPECIAL_START + 2)
#define TC_LEFT			(SPECIAL_START + 3)

#define TC_INSERT		(SPECIAL_START + 4)
#define TC_DELETE		(SPECIAL_START + 5)
#define TC_PPAGE		(SPECIAL_START + 6)
#define TC_NPAGE		(SPECIAL_START + 7)
#define TC_HOME			(SPECIAL_START + 8)
#define TC_END			(SPECIAL_START + 9)

#define TC_F1			(SPECIAL_START + 10)
#define TC_F2			(SPECIAL_START + 11)
#define TC_F3			(SPECIAL_START + 12)
#define TC_F4			(SPECIAL_START + 13)
#define TC_F5			(SPECIAL_START + 14)
#define TC_F6			(SPECIAL_START + 15)
#define TC_F7			(SPECIAL_START + 16)
#define TC_F8			(SPECIAL_START + 17)
#define TC_F9			(SPECIAL_START + 18)
#define TC_F10			(SPECIAL_START + 19)
#define TC_F11			(SPECIAL_START + 20)
#define TC_F12			(SPECIAL_START + 21)

#define TC_C_UP			(SPECIAL_START + 22)
#define TC_C_DOWN		(SPECIAL_START + 23)
#define TC_C_RIGHT		(SPECIAL_START + 24)
#define TC_C_LEFT		(SPECIAL_START + 25)
#define TC_C_HOME		(SPECIAL_START + 26)
#define TC_C_END		(SPECIAL_START + 27)

#define NUMKEYS			(SPECIAL_START + 28)

#define NUM_SPECIAL		(NUMKEYS - SPECIAL_START)

#endif
