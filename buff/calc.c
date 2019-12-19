/* calc.c - simple calculator
 * Copyright (C) 1988-2017 Sean MacLennan
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include "calc.h"

/** @addtogroup misc
 * @{
 */

/* \cond skip */
/** Main calc struct */
struct calc {
	char ops[MAX_OPS];	/**< Stack of operators. */
	int cur_op;		/**< Current pointer in operator stack. */

	union number nums[MAX_OPS]; /**< Stack of numbers. */
	int cur_num;		/**< Current pointer in numbers stack. */

	jmp_buf failed;		/**< Errors longjmp to here. */

	int is_float;		/**< Are we dealing with floats? */
};

/** Structure for the precedence values. */
struct values {
	char op; /**< The operator. */
	int val; /**< The precedence. */
};

/** Values for the precedence function `f'. */
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

/** Values for the precedence function `g'. */
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

/** Push an operator on the operator stack */
static void push_op(struct calc *c, char op)
{
	if (c->cur_op >= MAX_OPS)
		longjmp(c->failed, CALC_STACK_OVERFLOW);
	c->ops[c->cur_op++] = op;
}

/** Pop an operator from the operator stack */
static char pop_op(struct calc *c)
{
	if (c->cur_op < 0)
		longjmp(c->failed, CALC_SYNTAX_ERROR);
	return c->ops[--c->cur_op];
}

/** Return the current operator on the operator stack. Read only. */
static char top_op(struct calc *c)
{
	if (c->cur_op == 0)
		return '=';
	return c->ops[c->cur_op - 1];
}

/** Push an integer on the number stack */
static void push_num(struct calc *c, long num)
{
	if (c->cur_num >= MAX_OPS)
		longjmp(c->failed, CALC_STACK_OVERFLOW);
	c->nums[c->cur_num++].i = num;
}

/** Push a floating point number on the number stack */
static void push_float(struct calc *c, double num)
{
	if (c->cur_num >= MAX_OPS)
		longjmp(c->failed, CALC_STACK_OVERFLOW);
	c->nums[c->cur_num++].f = num;
}

/** Pop an integer or  floating point number from the number stack */
static union number pop_num(struct calc *c)
{
	if (c->cur_num == 0)
		longjmp(c->failed, CALC_SYNTAX_ERROR);

	return c->nums[--c->cur_num];
}

/** Low level lookup of the precedence value for an operator. */
static int lookup(struct calc *c, struct values *vals, char op)
{
	struct values *v;

	for (v = vals; v->op != op; ++v)
		if (!v->op)
			longjmp(c->failed, CALC_SYNTAX_ERROR);
	return v->val;
}

/** Is the current char an operator? */
static int is_op(char op)
{
	return strchr("*/%+-<>&^|", op) != NULL;
}

/** Precedence function `f'. */
static int calc_f(struct calc *c, char op)
{
	return lookup(c, fvals, op);
}

/** Precedence function `g'.
 * Only this function is ever looking at a number.
 * It reads the number, pushes it on the nums stack,
 * and replaces the number with the token `N' in the buffer.
 */
static int calc_g_num(struct calc *c, char **p)
{
	if (isdigit(**p) || **p == '.') {
		char *e;

		if (c->is_float)
			push_float(c, strtod(*p, &e));
		else {
			long num = strtol(*p, &e, 0);

			switch (*e) {
			case 't':
			case 'T':
				num *= 1024;
				/* fall thru */
			case 'g':
			case 'G':
				num *= 1024;
				/* fall thru */
			case 'm':
			case 'M':
				num *= 1024;
				/* fall thru */
			case 'k':
			case 'K':
				num *= 1024;
				++e;
				break;
			case 'p':
			case 'P':
				num *= 4096; /* page */
				++e;
			}
			push_num(c, num);
		}
		*p = e - 1;
		**p = 'N';
	}

	return lookup(c, gvals, **p);
}

/** Precedence function `g'. */
static int calc_g(struct calc *c, char op)
{
	return lookup(c, gvals, op);
}

/** Push the right value on the numbers stack. */
#define OP(op) do {					 \
		if (c->is_float)				 \
			push_float(c, one.f op two.f);		\
		else					 \
			push_num(c, one.i op two.i);			\
	} while (0)

/** Push an integer on the numbers stack verifying that we are not
 * doing floats.
 */
#define INT_OP(op) do {				       \
		if (c->is_float)			       \
			longjmp(c->failed, CALC_SYNTAX_ERROR); \
		else				       \
			push_num(c, one.i op two.i);			\
	} while (0)
/* \endcond */

/**  Simple calculator.
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
 *	&		bitwise and
 *	^		bitwise exclusive or
 *	|		bitwise or
 *
 * Supports the following floating point operations:
 *	( )	  grouping
 *	* /	  multiplication, division
 *	+  -  addition and subtraction
 * @param p The input string.
 * @param[out] result The output result.
 * @return 0 on success, one of the CALC_xxx defines on error.
 */
int calc(char *p, struct calc_result *result)
{
	struct calc calc, *c = &calc;
	int f_val, g_val, rc;

	/* A longjmp is called on error. */
	rc = setjmp(c->failed);
	if (rc)
		return rc;

	c->is_float = strchr(p, '.') != NULL;
	c->cur_op = c->cur_num = 0;

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
					union number one = pop_num(c);

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

	if (result) {
		result->is_float = c->is_float;
		result->result = pop_num(c);
	}

	return 0;
}
/* @} */
