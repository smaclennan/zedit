#include "z.h"

#if CEXPAND
typedef struct {
	char *match;
	int len;
	char *expand;
} Expand;

#define MAXEXPAND		100
static Expand CExpand[100] = {
	"if",		2,	"if(<%>)\n{\n`statement`;\n}",
	"elif",		4,	"else if(<%>)\n{\n`statement`;\n}",
	"else",		4,	"else\n{\n<%>\n}",
	"fori",		4,	"for(i = 0; i < <%>; ++i)\n{\n`statement`;\n}",
	"for",		3,	"for(<%>)\n{\n`statement`;\n}",
	"wh",		2,	"while(<%>)\n{\n`statement`;\n}",
	"sw",		2,	"switch(<%>)\n{\ncase `lit`:\n\t`code`\nbreak;\n}",
	"ca",		2,	"case ",
};
static int NExpand = 8;

/* We only expand lines of the form: <ws><keyword><ws> */
#define ISTOK(c)	(isalpha(c) || c == '#')

Proc Zcexpand()
{
	char word[STRMAX], *p;
	Mark was, start;

	Bmrktopnt(&was);
	Tobegline();
	Movepast(Iswhite, FORWARD);
	Bmrktopnt(&start);
	for(p = word; ISTOK(Buff()) && Bisbeforemrk(&was); Bmove1())
		*p++ = Buff();
	*p = '\0';
	Movepast(Iswhite, FORWARD);

	if(ISNL(Buff()) || Bisend())
	{
		int i;

		for(i = 0; i < NExpand; ++i)
			if(strncmp(word, CExpand[i].match, CExpand[i].len) == 0)
			{
				extern unsigned Cmd;
				char *p;
				
				Bpnttomrk(&start);
				Zdeleol();
				for(p = CExpand[i].expand; *p; ++p)
				{
					if(ISNL(Cmd = *p))
						Zcindent();
					else
						Zcinsert();
				}
				Bmrktopnt(&start);		/* leave at end */
				if(Bsearch("<%>", BACKWARD))
					Bdelete(3);
				else
					Bpnttomrk(&start);
				return;
			}
	}

	/* just insert the character */
	Bpnttomrk(&was);
	Zinsert();
}


/* Find and delete strings of the form `string` */
Proc Zcfindexpand()
{
	Mark was;
	
	Bmrktopnt(&was);
	if(Bcsearch('`'))
	{
		Mark start;
		Bmrktopnt(&start);
		Movepast(Istoken, FORWARD);
		if(Buff() == '`')
		{	/* found a match */
			Bmove1();
			Bmrktopnt(Curbuff->mark);
			Bpnttomrk(&start);
			Bmove(-1);
			Killtomrk(Curbuff->mark);
			return;
		}
	}
	Tbell();
	Bpnttomrk(&was);
}


/* Convert \t and \n into tabs and NLs */
static char *PreProcess(char *str)
{
	char *p, *s;
	
	for(p = s = strdup(str); *str; ++str)
		if(*str == '\\')
			switch(*++str)
			{
				case 'n': *p++ = '\n';	break;
				case 't': *p++ = '\t';	break;
				default:  *p++ = *str;	break;
			}
		else
			*p++ = *str;
	*p = '\0';
			
	return s;
}

static void ReadExpandFile(char *fname)
{
	FILE *fp;
	char buf[1024], *p, *s;
	int i;
	
	if((fp = fopen(fname, "r")) == 0) return;

	while(fgets(p = buf, 1024, fp))
	{	/* parse the line [#a-zA-Z][a-zA-Z]*[ \t]+[^ \t]+ */
		if(*p == '#')
			++p;
		else if(!isalpha(*p) && *p != '#')
			continue;
		while(isalpha(*p)) ++p;
		if(!isspace(*p)) continue;
		*p++ = '\0';
		while(isspace(*p)) ++p;
		if(*p == '\0') continue;
		s = p;
		if((p = strchr(p, '\n'))) *p = '\0';
		
		/* look for an existing entry */
		for(i = 0; i < NExpand; ++i)
		{
			if(strcmp(buf, CExpand[i].match) == 0)
			{	/* override an existing entry */
				/* SAM We leak memory here if we
				 * continuously overwrite the same
				 * entries. However, this saves having
				 * to make the defaults freeable.
				 */
				/* SAM We should also check if the new
				 * expansion matches the old one.
				 */
				break;
			}
		}
		if(i == NExpand)
		{
			if(i == MAXEXPAND)
				continue;
			else
			{	/* use a new slot */
				++NExpand;
			}
		}
		CExpand[i].match  = strdup(buf);
		CExpand[i].expand = PreProcess(s);
		CExpand[i].len    = strlen(buf);
	}
	
	fclose(fp);
}


Proc ZreadExpand()
{
	char fname[PATHMAX + 1];
	int i;

	for(i = FINDPATHS; i && (i = Findpath(fname, ZEFILE, i, TRUE)); --i)
		ReadExpandFile(fname);
}
#endif
