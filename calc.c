/* calc.c - trivial calculator
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

#if CALC
/*
 * EXTREMELY simple calculator.
 *
 * Handles signed decimal/octal/hexadecimal +, -, *, /.
 * Returns the value in decimal and hex.
 * Entering one number gives the conversion.
 * After an optional leading sign, a leading 0 indicates octal, 0x or 0X
 * indicates hex, else decimal.
 *
 * OR
 *
 * Floating point calculations + - * /.
 *
 * NOTE: does not follow order of operations.
 */

#if FLOATCALC
static void CalcFloat(char *str);
#endif

void Zcalc(void)
{
	char *ptr, op;
	long n1, n2;

	Arg = 0;
	if (Getarg("Calc: ", Calc_str, STRMAX))
		return;
#if FLOATCALC
	if (Argp || strchr(Calc_str, '.')) {
		CalcFloat(Calc_str);
		return;
	}
#endif
	ptr = Calc_str;
	n1 = strtol(ptr, &ptr, 0);
	while (*ptr) {
		while (isspace(*ptr))
			++ptr;
		op = *ptr;
		if (*ptr != '\0')
			++ptr;  /* save and skip op */
		n2 = strtol(ptr, &ptr, 0);
		switch (op) {
		case '+':
			n1 += n2;
			break;
		case '-':
			n1 -= n2;
			break;
		case '*':
			n1 *= n2;
			break;
		case '>':
			n1 >>= n2;
			break;
		case '<':
			n1 <<= n2;
			break;
		case '%':
			n1 %= n2;
			break;
		case '/':
			if (n2 == 0) {
				Echo("Divide by Zero");
				return;
			}
			n1 /= n2;
			break;
		default:
			Echo("Huh?");
			return;
		}
	}

	sprintf(PawStr, "= %ld (%lx)", n1, n1);
	Echo(PawStr);
}


#if FLOATCALC
static void CalcFloat(char *str)
{
	char op;
	double n1, n2;

	n1 = strtod(str, &str);
	while (*str) {
		while (isspace(*str))
			++str;
		op = *str;
		if (*str != '\0')
			++str;  /* save and skip op */
		n2 = strtod(str, &str);
		switch (op) {
		case '+':
			n1 += n2;
			break;
		case '-':
			n1 -= n2;
			break;
		case '*':
			n1 *= n2;
			break;
		case '/':
			if (n2 == 0.0) {
				Echo("Divide by Zero");
				return;
			}
			n1 /= n2;
			break;
		default:
			Echo("Huh?");
			return;
		}
	}

	sprintf(PawStr, "= %g", n1);
	Echo(PawStr);
}
#endif

#else
void Zcalc(void) { Tbell(); }
#endif /* CALC */
