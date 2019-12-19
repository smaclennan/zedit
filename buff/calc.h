/* calc.h - calc.c include file
 * Copyright (C) 1988-2017 Sean MacLennan <seanm@seanm.ca>
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

#ifndef _calc_h_
#define _calc_h_

/** @addtogroup misc
 * @{
 */

/** This is the maximum number of operations that can be outstanding
 * at one time. This mainly limits the number of nested brackets.
 */
#define MAX_OPS 10

#define CALC_STACK_OVERFLOW 1
#define CALC_SYNTAX_ERROR   2
#define CALC_OUT_OF_MEMORY  3
#define CALC_INTERNAL_ERROR 4

/** If you can't figure this one out, you probably shouldn't be
 * looking at this code.
 */
union number {
	long i;
	double f;
};

/** The result from the calc command if return is 0. */
struct calc_result {
	int is_float;
	union number result;
};

int calc(char *p, struct calc_result *result);
/* @} */

#endif
