/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

#if SPELL
#include <signal.h>

#define SPELLSTRING "<space>(skip)  A(ccept)  I(nsert)  R(eplace)  #"
#define PROMPT		"Replace with: "

static void Mclear ARGS((void));
static int ispell ARGS((FILE *, FILE *, char *, char *));

Proc Zspell()
{
	extern Byte Keys[];
	static char *argv[] = { "ispell", "-a", NULL };
	Buffer *was;
	Buffer *sbuff;
	Mark *emark;
	Mark *point, *mark;
	Byte cmd;
	char send[STRMAX + 2], buff[1024], *p, *e;
	char word[6][STRMAX + 1];
	int m, n;
	FILE *in, *out;
	
	/* save current Buff, Point, and Mark */
	was = Curbuff;
	point = Bcremrk();
	mark = Bcremrk();
	Mrktomrk(mark, Curbuff->mark);

	/* set Point and emark */
	if(Argp)
	{	/* use Region */
		emark = Bcremrk();
		Mrktomrk(emark, Curbuff->mark);
		if(Bisaftermrk(emark)) Bswappnt(emark);
	}
	else
	{	/* use entire buffer */
		Btoend();
		Bmove(-1);
		emark = Bcremrk();
		Btostart();
	}

	if((sbuff = Cmakebuff(SPELLBUFF, NULL)) && Invoke(sbuff, argv))
	{
		in = fdopen(sbuff->in_pipe, "r");
		out = sbuff->out_pipe;

		/* make the paw 3 lines */
		Bswitchto(was);
		Resize(-2);
		Refresh();
		Mclear();
		Echo("Checking...");
		while(Bisbeforemrk(emark))
		{	/* get the next word */
			Moveto(Isalpha, FORWARD);
			if(Bisend()) break;	/* incase no alphas left */
			Bmrktopnt(Curbuff->mark);
			for(p = send + 1; Isalpha() && !Bisend(); ++p, Bmove1())
				*p = Buff();
			*p++ = '\n'; *p = '\0';

			Mclear();

			/* process the word */
			if((n = ispell(in, out, send + 1, buff)) > 0)
			{
				m = -1;
				switch(*buff)
				{
				case '*':		/* ok */
				case '+':		/* ok with suffix removed */
					break;
				
				case '&':		/* close matches */
					Pset(Tmaxrow(), 0, PNUMCOLS);
					for(m = 0, p = buff + 2; m < 6; ++m)
					{
						if(!(e = strchr(p, ' ')) && !(e = strchr(p, '\n')))
							break;
						if(e == p) break;
						*e++ = '\0';
						word[m][0] = m + '0';
						word[m][1] = ' ';
						strcpy(&word[m][2], p);
						Pout(word[m], FALSE);
						p = e;
					}
					/* 'm' is now the number of matches - drop thru */

				case '#':		/* no match */
					Echo(SPELLSTRING);
					Refresh();		/* update mark */
					switch(cmd = Tgetcmd())
					{
						case ' ':		/* skip it */
							break;

						case 'A':		/* accept */
						case 'a':
							*send = '@';
							ispell(in, out, send, buff);
							break;
							
						case 'I':		/* insert in dictionary */
						case 'i':
							*send = '*';
							ispell(in, out, send, buff);
							break;
							
						case 'R':		/* replace */
						case 'r':
							*word[0] = '\0';
							if(Getarg(PROMPT, word[0], STRMAX) == ABORT)
							{
								Bswappnt(Curbuff->mark);
								continue;
							}
							sreplace(word[0]);
							break;

						default:
							if(isdigit(cmd) && (n = cmd - '0') < m)
								sreplace(word[n] + 2);
							else
							{
								Tbell();
								if(Keys[cmd] == ZABORT) goto abort;
							}
							break;
					}
					break;
				
				default:		/* invalid */
					Error("Unable to start ispell");
					goto abort;
				}
			}
		}
abort:
		fclose(in);		/* from fdopen */
		Delbuff(sbuff);
		Bswitchto(was);
		Bpnttomrk(point);
		Mrktomrk(Curbuff->mark, mark);
		Unmark(point);
		Unmark(mark);
		Unmark(emark);
		Resize(2);
	}
}


static void Mclear()
{
	/* clear the matches area */
	Clrcol[Rowmax] = Clrcol[Rowmax + 1] = COLMAX + 1;
	Tsetpoint(Rowmax, 0);
	Tcleol();
	Tsetpoint(Rowmax + 1, 0);
	Tcleol();
}


static int ispell(in, out, send, receive)
FILE *in, *out;
char *send, *receive;
{
	fputs(send, out);
	fflush(out);
	return fgets(receive, 1024, in) ? strlen(receive) : 0;
}


/* replace with new, assumes Region contains old word */
void sreplace(new)
char *new;
{
	Bdeltomrk(Curbuff->mark);
	Binstr(new);
}

		
int Isalpha()
{
	return(isalpha(Buff()));
}
#else
Proc Zspell() { Tbell(); }
#endif
