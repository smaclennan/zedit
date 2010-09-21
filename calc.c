/* calc.c - simple calculator
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

#include "z.h"

/*  Simple calculator.
 *
 * The calculator is based on the operator-precedence parsing algorithm
 * from "Compilers Principles, Techniques, and Tools"
 * by Alfred V. Aho, Ravi Sethi, and Jeffery D. Ullman.
 *
 * Supports the following integer operations:
 * 	( )	grouping
 * 	* / %	multiplication, division, modulo
 * 	+  -	addition and subtraction
 * 	<< >>	arithmetic shift left and right
 * 	&	bitwise and
 * 	^	bitwise exclusive or
 * 	|	bitwise or
 *
 * Supports the following floating point operations:
 * 	( )	grouping
 * 	* /	multiplication, division
 * 	+  -	addition and subtraction
 */

struct values {
	char op;
	int val;
};

/* Values for the precedence function `f'. */
static struct values fvals[] = {
	{ '*', 12 }, { '/', 12 }, { '%',  12 },
	{ '+', 10 }, { '-', 10 },
	{ '<', 8 }, { '>', 8 },
	{ '&', 6 }, { '^', 4 }, { '|', 2 },
	{ '(', 0 }, { ')', 14 },
	{ 'N', 14 }, /* number */
	{ '=', 0 },  /* terminator */
	{ '\0', 0 }
};

/* Values for the precedence function `g'. */
static struct values gvals[] = {
	{ '*', 11 }, { '/', 11 }, { '%',  11 },
	{ '+', 9 }, { '-', 9 },
	{ '<', 7 }, { '>', 7 },
	{ '&', 5 }, { '^', 3 }, { '|', 1 },
	{ '(', 13 }, { ')', 0 },
	{ 'N', 13 }, /* number */
	{ '=', 0 },   /* terminator */
	{ '\0', 0 }
};

#define MAX_OPS 10
static char ops[MAX_OPS];
static int cur_op;

#define MAX_NUMS 10
static union number {
	long i;
	double f;
} nums[MAX_OPS];
static int cur_num;
static int is_float;

static int max_num, max_op; /* SAM DBG */

static jmp_buf failed;

#define STACK_OVERFLOW 1
#define SYNTAX_ERROR   2

static void push_op(char op)
{
	if (cur_op >= MAX_OPS)
		longjmp(failed, STACK_OVERFLOW);
	ops[cur_op++] = op;
	if (cur_op > max_op)
		max_op = cur_op;
}

static char pop_op(void)
{
	if (cur_op < 0)
		longjmp(failed, SYNTAX_ERROR);
	return ops[--cur_op];
}

static char top_op(void)
{
	if (cur_op == 0)
		return '=';
	return ops[cur_op - 1];
}

static void push_num(long num)
{
	if (cur_num >= MAX_NUMS)
		longjmp(failed, STACK_OVERFLOW);
	nums[cur_num++].i = num;
	if (cur_num > max_num)
		max_num = cur_num;
}

static void push_float(double num)
{
	if (cur_num >= MAX_NUMS)
		longjmp(failed, STACK_OVERFLOW);
	nums[cur_num++].f = num;
}

static union number pop_num(void)
{
	if (cur_num == 0)
		longjmp(failed, SYNTAX_ERROR);

	return nums[--cur_num];
}

static int lookup(struct values *vals, char op)
{
	struct values *v;

	for (v = vals; v->op != op; ++v)
		if (!v->op)
			longjmp(failed, SYNTAX_ERROR);
	return v->val;
}

static int is_op(char op)
{
	return strchr("*/%+-<>&^|", op) != NULL;
}

/* Precedence function `f'. */
static int calc_f(char op)
{
	return lookup(fvals, op);
}

/* Precedence function `g'.
 * Only the `g' function is ever looking at a number.
 * It reads the number, pushes it on the nums stack,
 * and replaces the number with the token `N' in the buffer.
 */
static int calc_g_num(char **p)
{
	if (isdigit(**p) || **p == '.') {
		char *e;

		if (is_float)
			push_float(strtod(*p, &e));
		else
			push_num(strtol(*p, &e, 0));
		*p = e - 1;
		**p = 'N';
	}

	return lookup(gvals, **p);
}

static int calc_g(char op)
{
	return lookup(gvals, op);
}

#define OP(op) do {					 \
		if (is_float)				 \
			push_float(one.f op two.f);	 \
		else					 \
			push_num(one.i op two.i);	 \
	} while (0)


#define INT_OP(op) do {				       \
		if (is_float) 			       \
			longjmp(failed, SYNTAX_ERROR); \
		else				       \
			push_num(one.i op two.i);      \
	} while (0)

void Zcalc(void)
{
	int f_val, g_val, n;
	char op, str[STRMAX], *p = str;

	Arg = 0;
	if (Getarg("Calc: ", Calc_str, STRMAX - 1))
		return;

	/* We modify the string, leave Calc_str alone */
	strcpy(str, Calc_str);
	strcat(str, "=");

	is_float = strchr(Calc_str, '.') != NULL;

	cur_op = cur_num = 0;

	/* A longjmp is called on error. */
	n = setjmp(failed);
	if (n == 1) {
		Error("Stack overflow.");
		return;
	} else if (n == 2) {
		Error("Syntax error.");
		return;
	}

	/* Continue until all input parsed and command stack empty. */
	while (*p != '=' || top_op() != '=') {
		while (isspace(*p))
			++p;

		/* special case for shifts */
		if (*p == '<' && *(p + 1) == '<')
			++p;
		else if (*p == '>' && *(p + 1) == '>')
			++p;

		f_val = calc_f(top_op());
		g_val = calc_g_num(&p);
		if (g_val < 0)
			return;

		if (f_val <= g_val) {
			/* shift */
			push_op(*p);
			if (*p != '=')
				++p;
		} else {
			/* reduce */
			do {
				op = pop_op();
				if (is_op(op)) {
					union number two = pop_num();
					union number  one = pop_num();

					switch (op) {
					case '*':
						OP(*);
						break;
					case '/':
						OP(/);
						break;
					case '%':
						INT_OP(%);
						break;
					case '+':
						OP(+);
						break;
					case '-':
						OP(-);
						break;
					case '>':
						INT_OP(>>);
						break;
					case '<':
						INT_OP(<<);
						break;
					case '&':
						INT_OP(&);
						break;
					case '^':
						INT_OP(^);
						break;
					case '|':
						INT_OP(|);
						break;
					}
				}

				f_val = calc_f(top_op());
				g_val = calc_g(op);
			} while (f_val >= g_val);
		}
	}

	if (is_float)
		sprintf(PawStr, "= %g", pop_num().f);
	else {
		long n = pop_num().i;
		sprintf(PawStr, "= %ld (%lx)", n, n);
	}

	Echo(PawStr);

	Dbg("Max op %d num %d (%s)\n", max_op, max_num, Calc_str);
}
