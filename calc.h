/* calc.h - calc.c include file
 * Copyright (C) 1988-2015 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#define CALC_STACK_OVERFLOW 1
#define CALC_SYNTAX_ERROR   2
#define CALC_INTERNAL_ERROR 3

/** If you can't figure this one out, you probably shouldn't be
 * looking at this code.
 */
union number {
	long i;
	double f;
};

struct calc {
	char *ops;			/**< Stack of operators. */
	int cur_op;			/**< Current pointer in operator stack. */
	int max_ops;		/**< Maximum number of operators/numbers. */

	union number *nums; /**< Stack of numbers. */
	int cur_num;		/**< Current pointer in numbers stack. */

	jmp_buf failed;		/**< Errors longjmp to here. */

	int is_float;		/**< Are we dealing with floats? */
	union number result;/**< The result. */
};

int calc(struct calc *c, char *p);

#endif
