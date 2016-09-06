/* keys.h - Zedit key code defines
 * Copyright (C) 1988-2016 Sean MacLennan
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

/* WIN32 needs these to fit in a byte */
#define TC_UP			CX('a')
#define TC_DOWN			CX('b')
#define TC_RIGHT		CX('c')
#define TC_LEFT			CX('d')

#define TC_INSERT		CX('e')
#define TC_DELETE		CX('f')
#define TC_PGUP			CX('g')
#define TC_PGDOWN		CX('h')
#define TC_HOME			CX('i')
#define TC_END			CX('j')

#define TC_F1			CX('k')
#define TC_F2			CX('l')
#define TC_F3			CX('m')
#define TC_F4			CX('n')
#define TC_F5			CX('o')
#define TC_F6			CX('p')
#define TC_F7			CX('q')
#define TC_F8			CX('r')
#define TC_F9			CX('s')
#define TC_F10			CX('t')
#define TC_F11			CX('u')
#define TC_F12			CX('v')

#define TC_C_HOME		CX('w')
#define TC_C_END		CX('x')

/* Unknown key - always bound to Znotimpl */
#define TC_UNKNOWN		CX('z')

#define KEY_MASK		0x00ffffff
#define SPECIAL_START	TC_UP
#define SPECIAL_END		TC_C_END
#define NUM_SPECIAL		(SPECIAL_END - SPECIAL_START + 1)

/* 128 ASCII + 128 meta + 128 C-X */
#define NUMKEYS			(128 + 128 + 128)

/* kbd.c */
Byte tgetkb(void);
void tungetkb(int n);
Byte tpeek(int offset);
int tgetcmd(void);
bool tkbrdy(void);
bool tdelay(int ms);
void termcap_keys(void);

#endif
