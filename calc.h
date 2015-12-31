#ifndef _calc_h_
#define _calc_h_

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#define CALC_STACK_OVERFLOW 1
#define CALC_SYNTAX_ERROR   2
#define CALC_INTERNAL_ERROR 3

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
