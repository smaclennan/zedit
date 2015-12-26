/* calc.c - simple calculator
 * Copyright (C) 1988-2013 Sean MacLennan
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
 *	( )	grouping
 *	* / %	multiplication, division, modulo
 *	+  -	addition and subtraction
 *	<< >>	arithmetic shift left and right
 *	&	bitwise and
 *	^	bitwise exclusive or
 *	|	bitwise or
 *
 * Supports the following floating point operations:
 *	( )	grouping
 *	* /	multiplication, division
 *	+  -	addition and subtraction
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

#define STACK_OVERFLOW 1
#define SYNTAX_ERROR   2

static void push_op(struct calc *c, char op)
{
	if (c->cur_op >= c->max_ops)
		longjmp(c->failed, STACK_OVERFLOW);
	c->ops[c->cur_op++] = op;
}

static char pop_op(struct calc *c)
{
	if (c->cur_op < 0)
		longjmp(c->failed, SYNTAX_ERROR);
	return c->ops[--c->cur_op];
}

static char top_op(struct calc *c)
{
	if (c->cur_op == 0)
		return '=';
	return c->ops[c->cur_op - 1];
}

static void push_num(struct calc *c, int num)
{
	if (c->cur_num >= c->max_ops)
		longjmp(c->failed, STACK_OVERFLOW);
	c->nums[c->cur_num++].i = num;
}

static void push_float(struct calc *c, double num)
{
	if (c->cur_num >= c->max_ops)
		longjmp(c->failed, STACK_OVERFLOW);
	c->nums[c->cur_num++].f = num;
}

static union number pop_num(struct calc *c)
{
	if (c->cur_num == 0)
		longjmp(c->failed, SYNTAX_ERROR);

	return c->nums[--c->cur_num];
}

static int lookup(struct calc *c, struct values *vals, char op)
{
	struct values *v;

	for (v = vals; v->op != op; ++v)
		if (!v->op)
			longjmp(c->failed, SYNTAX_ERROR);
	return v->val;
}

static int is_op(char op)
{
	return strchr("*/%+-<>&^|", op) != NULL;
}

/* Precedence function `f'. */
static int calc_f(struct calc *c, char op)
{
	return lookup(c, fvals, op);
}

/* Precedence function `g'.
 * Only the `g' function is ever looking at a number.
 * It reads the number, pushes it on the nums stack,
 * and replaces the number with the token `N' in the buffer.
 */
static int calc_g_num(struct calc *c, char **p)
{
	if (isdigit(**p) || **p == '.') {
		char *e;

		if (c->is_float)
			push_float(c, strtod(*p, &e));
		else
			push_num(c, strtol(*p, &e, 0));
		*p = e - 1;
		**p = 'N';
	}

	return lookup(c, gvals, **p);
}

static int calc_g(struct calc *c, char op)
{
	return lookup(c, gvals, op);
}

#define OP(op) do {					 \
		if (c->is_float)				 \
			push_float(c, one.f op two.f);		\
		else					 \
			push_num(c, one.i op two.i);			\
	} while (0)


#define INT_OP(op) do {				       \
		if (c->is_float)			       \
			longjmp(c->failed, SYNTAX_ERROR); \
		else				       \
			push_num(c, one.i op two.i);			\
	} while (0)

int calc(struct calc *c, char *p)
{
	int f_val, g_val;

	c->is_float = strchr(p, '.') != NULL;
	c->cur_op = c->cur_num = 0;

	/* A longjmp is called on error. */
	switch (setjmp(c->failed)) {
	case 0:
		break;
	case STACK_OVERFLOW:
		error("Stack overflow.");
		return -1;
	case SYNTAX_ERROR:
		error("Syntax error.");
		return -1;
	default:
		error("Calc internal error.");
		return -1;
	}

	/* Continue until all input parsed and command stack empty. */
	while (*p != '=' || top_op(c) != '=') {
		while (isspace(*p))
			++p;

		/* special case for shifts */
		if (*p == '<' && *(p + 1) == '<')
			++p;
		else if (*p == '>' && *(p + 1) == '>')
			++p;

		f_val = calc_f(c, top_op(c));
		g_val = calc_g_num(c, &p);
		if (g_val < 0)
			return -1;

		if (f_val <= g_val) {
			/* shift */
			push_op(c, *p);
			if (*p != '=')
				++p;
		} else {
			/* reduce */
			do {
				int op = pop_op(c);
				if (is_op(op)) {
					union number two = pop_num(c);
					union number  one = pop_num(c);

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

				f_val = calc_f(c, top_op(c));
				g_val = calc_g(c, op);
			} while (f_val >= g_val);
		}
	}

	c->result = pop_num(c);
	return 0;
}
