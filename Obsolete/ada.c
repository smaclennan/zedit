#include "z.h"

static int AdaSpecial ARGS((void));

Proc Zadaindent()
{
	Mark *tmark;
	int width;

	if( (Curbuff->bmode & OVERWRITE) )
		Bcsearch( NL );
	else
	{
		tmark = Bcremrk();
		Tobegline();
		Movepast(Iswhite, FORWARD);
		width = Bgetcol(TRUE, 0) + AdaSpecial();
		Bpnttomrk(tmark);
		Unmark(tmark);
		Binsert(NL);
		Tindent(width);
	}
}


/* SAM NOT YET */

/* Check for special ada words */
static int AdaSpecial()
{
#if 0
	char word[11];
	int i;
	
	switch(Buff())
	{	/* fast check of first character */
		case 'b':	/* begin */
		case 'i':	/* if */
		case 'l':	/* loop */
			for(i = 0; isalpha(Buff()) && i < 10 && !Bisend(); ++i, Bmove1())
				word[i] = TOLOWER(Buff());
			word[i] = '\0';
			if(strcmp(word, "begin") == 0 || strcmp(word, "if") == 0 ||
			   strcmp(word, "loop") == 0)
				return Tabsize;
			break;

#if 1
		case 'e':	/* end */
			for(i = 0; isalpha(Buff()) && i < 10 && !Bisend(); ++i, Bmove1())
				word[i] = TOLOWER(Buff());
			word[i] = '\0';
			if(strcmp(word, "end") == 0)
			{
				Tobegline();
				if(Buff() == '\t') Bdelete(1);
				return -Tabsize;
			}
			break;
#endif
	}
#endif
	return 0;
}
