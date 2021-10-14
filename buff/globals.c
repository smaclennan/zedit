/* buffer global variables
 * Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Default #bsetmod function. Does nothing ;) */
static void dummy_bsetmod(struct buff *buff) {}
/** If you need to know when the buffer is modified, this callback
 * will get called to notify you. By default it is set to a dummy
 * function that does nothing.
 */
void (*bsetmod)(struct buff *buff) = dummy_bsetmod;

#ifdef GLOBAL_MARKS
/** Global mark list. The buffer code keeps the marks in this list
 * up to date.
 */
struct mark *Marklist;	/* the marks list tail */
#endif
/* @} */


/* \cond skip */
/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x)	#x
#define __stringify(x)	__stringify_1(x)

/* The marker is for `strings libbuff.a | fgrep Version' */
const char *libbuff_marker =
	"Version-" __stringify(LIBBUFF_MAJOR) "." __stringify(LIBBUFF_MINOR);

const char *libbuff_version =
	__stringify(LIBBUFF_MAJOR) "." __stringify(LIBBUFF_MINOR);
/* \endcond */
