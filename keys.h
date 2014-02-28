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

#define CX(n)	((n) + 128)
#define M(n)	((n) + 256)

#define TC_UP			M('a')
#define TC_DOWN			M('b')
#define TC_RIGHT		M('c')
#define TC_LEFT			M('d')

#define TC_INSERT		M('e')
#define TC_DELETE		M('f')
#define TC_PGUP			M('g')
#define TC_PGDOWN		M('h')
#define TC_HOME			M('i')
#define TC_END			M('j')

#define TC_F1			M('k')
#define TC_F2			M('l')
#define TC_F3			M('m')
#define TC_F4			M('n')
#define TC_F5			M('o')
#define TC_F6			M('p')
#define TC_F7			M('q')
#define TC_F8			M('r')
#define TC_F9			M('s')
#define TC_F10			M('t')
#define TC_F11			M('u')
#define TC_F12			M('v')

#define TC_C_HOME		M('w')
#define TC_C_END		M('x')

/* Unknown key - always bound to Znotimpl */
#define TC_UNKNOWN		M('z')

#define KEY_MASK		0x00ffffff
#define SPECIAL_START		TC_UP
#define SPECIAL_END		TC_C_END
#define NUM_SPECIAL		(SPECIAL_END - SPECIAL_START + 1)

/* 128 ASCII + 128 meta + 128 C-X */
#define NUMKEYS			(128 + 128 + 128)

#define is_special(cmd) (cmd >= SPECIAL_START && cmd <= SPECIAL_END)

#endif
