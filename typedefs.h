/* typedefs.h - Zedit typedefs
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

/* Ok, I admit it.... this is a hack to get around the checkpatch
 * typedef errors. Easier to just put them all in one file and ignore
 * them :(
 */

/* from buff.h */
typedef struct page Page;
typedef struct mark Mark;
typedef struct buff Buffer;
typedef struct wdo WDO;

/* from old global.h */
#ifndef _XtIntrinsic_h
typedef int Boolean;
#endif

typedef unsigned char Byte;
