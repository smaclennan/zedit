/* Copyright (C) 1988-2017 Sean MacLennan <seanm@seanm.ca> */

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
