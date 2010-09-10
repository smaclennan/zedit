#include "z.h"

/*	abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ
 *	nopqrstuvwxyzabcdefghijklmNOPQRSTUVWXYZABCDEFGHIJKLM
 */

Proc Zrot13()
{
	Mark pmark, end;
	
	Bmrktopnt(&pmark);
	
	if(Argp)
	{	/* use region */
		if(Bisaftermrk(Curbuff->mark))
		{
			Bmrktopnt(&end);
			Bpnttomrk(Curbuff->mark);
		}
		else
			Mrktomrk(&end, Curbuff->mark);
	}
	else
	{	/* use entire buffer */
		Btoend();
		Bmrktopnt(&end);
		Btostart();
	}

	for( ; !Bisatmrk(&end); Bmove1())
		     if(*Curcptr >= 'a' && *Curcptr <= 'm') *Curcptr += 13;
		else if(*Curcptr >= 'A' && *Curcptr <= 'M') *Curcptr += 13;
		else if(*Curcptr >= 'n' && *Curcptr <= 'z') *Curcptr -= 13;
		else if(*Curcptr >= 'N' && *Curcptr <= 'Z') *Curcptr -= 13;

	Bpnttomrk(&pmark);
	
	Redisplay();
}
