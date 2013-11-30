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

#define TC_UP			(128 + 'a')
#define TC_DOWN			(128 + 'b')
#define TC_RIGHT		(128 + 'c')
#define TC_LEFT			(128 + 'd')

#define TC_INSERT		(128 + 'e')
#define TC_DELETE		(128 + 'f')
#define TC_PPAGE		(128 + 'g')
#define TC_NPAGE		(128 + 'h')
#define TC_HOME			(128 + 'i')
#define TC_END			(128 + 'j')

#define TC_F1			(128 + 'k')
#define TC_F2			(128 + 'l')
#define TC_F3			(128 + 'm')
#define TC_F4			(128 + 'n')
#define TC_F5			(128 + 'o')
#define TC_F6			(128 + 'p')
#define TC_F7			(128 + 'q')
#define TC_F8			(128 + 'r')
#define TC_F9			(128 + 's')
#define TC_F10			(128 + 't')
#define TC_F11			(128 + 'u')
#define TC_F12			(128 + 'v')

#define KEY_MASK		0x003fffff
#define SPECIAL_START		TC_UP
#define SPECIAL_END		TC_F12
#define NUM_SPECIAL		(SPECIAL_END - SPECIAL_START + 1)

/* 128 ASCII + 128 meta + 128 C-X */
#define NUMKEYS			(128 + 128 + 128)

static inline bool is_special(int cmd)
{
	return cmd >= SPECIAL_START && cmd <= SPECIAL_END;
}

#endif
