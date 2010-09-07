%{
#include <stdio.h>
#include <string.h>
#include "global.h"

char *yyptr;	/* SAM can't use yysptr */
int yycol;

/* SAM Is this the right one to call? Or do we need TRUE? */
int Width ARGS((char ch, int col, int adjust));
#define Twidth(ch)			Width(ch, yycol, 0)

void yysetinput(s)
char *s;
{
	for(yyptr = yysbuf; *s && *s != '\376'; ++s, ++yyptr) *yyptr = *s;
	*yyptr++ = '\0'; *yyptr = '\0';
	yyptr = yysbuf;
	yycol = 0;
}

/* input and unput must match in adascan.l and cscan.l */
#undef input
#undef unput
#if 0
#define input()		(yywas = yycol, yycol += Twidth(*yyptr), *yyptr++)
#define unput(c)	(yycol = yywas, *--yyptr = (c))
#else
#define MAXNEST		7		/* SAM seems to nest a maximum depth 0f 3 */
static int nestring[MAXNEST + 1], nested = 0;


char input()
{
	nestring[nested] = yycol;
	nested = (nested + 1) & MAXNEST;
	yycol += Twidth(*yyptr);
	return *yyptr++;
}


void unput(c)
int c;
{
	nested = (nested - 1) & MAXNEST;
	yycol = nestring[nested];
	*--yyptr = c;
}
#endif


void keyword(), skipword();
static void comment();
%}

%%
"--"		{ comment(); }

"abort"		{ keyword("abort"); }
"abs"		{ keyword("abs"); }
"accept"	{ keyword("accept"); }
"access"	{ keyword("access"); }
"all"		{ keyword("all"); }
"and"		{ keyword("and"); }
"array"		{ keyword("array"); }
"at"		{ keyword("at"); }
"begin"		{ keyword("begin"); }
"body"		{ keyword("body"); }
"case"		{ keyword("case"); }
"constant"	{ keyword("constant"); }
"declare"	{ keyword("declare"); }
"delay"		{ keyword("delay"); }
"delta"		{ keyword("delta"); }
"digits"	{ keyword("digits"); }
"do"		{ keyword("do"); }
"else"		{ keyword("else"); }
"elsif"		{ keyword("elsif"); }
"end"		{ keyword("end"); }
"entry"		{ keyword("entry"); }
"exception"	{ keyword("exception"); }
"exit"		{ keyword("exit"); }
"for"		{ keyword("for"); }
"function"	{ keyword("function"); }
"generic"	{ keyword("generic"); }
"goto"		{ keyword("goto"); }
"if"		{ keyword("if"); }
"in"		{ keyword("in"); }
"is"		{ keyword("is"); }
"limited"	{ keyword("limited"); }
"loop"		{ keyword("loop"); }
"mod"		{ keyword("mod"); }
"new"		{ keyword("new"); }
"not"		{ keyword("not"); }
"null"		{ keyword("null"); }
"of"		{ keyword("of"); }
"or"		{ keyword("or"); }
"others"	{ keyword("others"); }
"out"		{ keyword("out"); }
"package"	{ keyword("package"); }
"pragma"	{ keyword("pragma"); }
"private"	{ keyword("private"); }
"procedure"	{ keyword("procedure"); }
"raise"		{ keyword("raise"); }
"range"		{ keyword("range"); }
"record"	{ keyword("record"); }
"rem"		{ keyword("rem"); }
"renames"	{ keyword("renames"); }
"return"	{ keyword("return"); }
"reverse"	{ keyword("reverse"); }
"select"	{ keyword("select"); }
"separate"	{ keyword("separate"); }
"subtype"	{ keyword("subtype"); }
"task"		{ keyword("task"); }
"terminate"	{ keyword("terminate"); }
"then"		{ keyword("then"); }
"type"		{ keyword("type"); }
"use"		{ keyword("use"); }
"when"		{ keyword("when"); }
"while"		{ keyword("while"); }
"with"		{ keyword("with"); }
"xor"		{ keyword("xor"); }

[a-zA-Z0-9]	{ skipword(); }

.			{ /* ignore bad characters */ }

%%

yywrap()
{
	return(1);
}


static void comment()
{	/* skip to end-of-line */
	int c;

	while((c = input()) != '\n' && c != 0) ;
}


void keyword(s)
char *s;
{
	extern int Tlrow;
	int c, len;

	if(isalpha(*s))
		if(isalnum((c = input())) || c == '_')
		{	/* not a complete keyword (i.e. integer instead of in) */
			skipword();
			return;
		}
		else
			unput(c);

	len = strlen(s);
	BoldWord(Tlrow, yycol - len, s, len);
}


void skipword()
{	/* skip word */
	int c;
	
	while((isalnum(c = input()) || c == '_') && c != 0) ;
	if(c) unput(c);
}
