#ifndef _calc_h_
#define _calc_h_

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
