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
	char *ops;
	int cur_op;
	int max_ops;

	union number *nums;
	int cur_num;

	jmp_buf failed;

	int is_float;
	union number result;
};

int calc(struct calc *c, char *p);

#endif
