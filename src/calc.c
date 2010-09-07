/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

/*
EXTREMELY simple calculator.

Handles signed decimal/octal/hexadecimal +, -, *, /.
Returns the value in decimal and hex.
Entering one number gives the conversion.
After an optional leading sign, a leading 0 indicates octal, 0x or 0X
indicates hex, else decimal.

OR

Floating point calculations + - * /.

NOTE: does not follow order of operations.
*/

#if FLOATCALC
static void CalcFloat ARGS((char *str));
#endif

Proc Zcalc()
{
	extern long strtol();
	extern char Calc_str[];
	char *ptr, op;
	long n1, n2;
	
	Arg = 0;
	if(Getarg("Calc: ", Calc_str, STRMAX)) return;
#if FLOATCALC
	if(Argp || strchr(Calc_str, '.'))
	{
		CalcFloat(Calc_str);
		return;
	}
#endif
	ptr = Calc_str;
	n1 = strtol(ptr, &ptr, 0);
	while(*ptr)
	{
		while(isspace(*ptr)) ++ptr;
		if((op = *ptr) != '\0') ++ptr;  /* save and skip op */
		n2 = strtol(ptr, &ptr, 0);
		switch( op )
		{
			case '+': n1 += n2; 	break;
			case '-': n1 -= n2;		break;
			case '*': n1 *= n2;		break;
			case '>': n1 >>= n2;	break;
			case '<': n1 <<= n2;	break;
			case '%': n1 %= n2;		break;
			case '/':
				if(n2 == 0)
				{
					Echo("Divide by Zero");
					return;
				}
				else
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
static void CalcFloat(str)
char *str;
{
	char op;
	double n1, n2;

	n1 = strtod(str, &str);
	while(*str)
	{
		while(isspace(*str)) ++str;
		if((op = *str) != '\0') ++str;  /* save and skip op */
		n2 = strtod(str, &str);
		switch(op)
		{
			case '+': n1 += n2; 	break;
			case '-': n1 -= n2;		break;
			case '*': n1 *= n2;		break;
			case '/':
				if(n2 == 0.0)
				{
					Echo("Divide by Zero");
					return;
				}
				else
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
