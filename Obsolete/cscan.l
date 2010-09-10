%{
#include <stdio.h>
#include <string.h>

extern char *yyptr;
extern int yycol, yywas;

/* SAM Is this the right one to call? Or do we need TRUE? */
#define Twidth(ch)			Width(ch, yycol, 0)

#if 0
yysetinput(s)
char *s;
{
	for(yyptr = yysbuf; *s && *s != '\376'; ++s, ++yyptr) *yyptr = *s;
	*yyptr++ = '\0'; *yyptr = '\0';
	yyptr = yysbuf;
	yycol = 0;
}
#endif

/* input and unput must match in adascan.l and cscan.l */
#undef input
#undef unput
#if 0
#define input()		(yywas = yycol, yycol += Twidth(*yyptr), *yyptr++)
#define unput(c)	(yycol = yywas, *--yyptr = (c))
#endif

void keyword(), skipword();
static void comment();
%}

%%
"/*"				{ comment(0); }
"*/"				{ comment(1); }

"auto"				{ keyword("auto"); }
"break"				{ keyword("break"); }
"case"				{ keyword("case"); }
"char"				{ keyword("char"); }
"continue"			{ keyword("continue"); }
"default"			{ keyword("default"); }
"do"				{ keyword("do"); }
"double"			{ keyword("double"); }
"#else"				{ skipword(); }
"else"				{ keyword("else"); }
"entry"				{ keyword("entry"); /* not in c++ */ }
"extern"			{ keyword("extern"); }
"float"				{ keyword("float"); }
"for"				{ keyword("for"); }
"goto"				{ keyword("goto"); }
"#if"				{ skipword(); }
"if"				{ keyword("if"); }
"int"				{ keyword("int"); }
"long"				{ keyword("long"); }
"register"			{ keyword("register"); }
"return"			{ keyword("return"); }
"short"				{ keyword("short"); }
"sizeof"			{ keyword("sizeof"); }
"static"			{ keyword("static"); }
"struct"			{ keyword("struct"); }
"switch"			{ keyword("switch"); }
"typedef"			{ keyword("typedef"); }
"union"				{ keyword("union"); }
"unsigned"			{ keyword("unsigned"); }
"while"				{ keyword("while"); }

"const"				{ keyword("const"); }
"enum"				{ keyword("enum"); 	}
"void"				{ keyword("void"); 	}
"volatile"			{ keyword("volatile");}

"asm"				{ keyword("asm"); }
"catch"				{ keyword("catch"); }
"class"				{ keyword("class"); }
"delete"			{ keyword("delete"); }
"friend"			{ keyword("friend"); }
"inline"			{ keyword("inline"); }
"new"				{ keyword("new"); }
"operator"			{ keyword("operator"); }
"private"			{ keyword("private"); }
"protected"			{ keyword("protected"); }
"public"			{ keyword("public"); }
"signed"			{ keyword("signed"); }
"template"			{ keyword("template"); }
"this"				{ keyword("this"); }
"throw"				{ keyword("throw"); }
"try"				{ keyword("try"); }
"virtual"			{ keyword("virtual"); }

"bool"				{ keyword("bool"); }
"const_class"		{ keyword("const_class"); }
"dynamic_cast"		{ keyword("dynamic_cast"); }
"explicit"			{ keyword("explicit"); }
"false"				{ keyword("false"); }
"mutable"			{ keyword("mutable"); }
"namespace"			{ keyword("namespace"); }
"overload"			{ keyword("overload"); }
"reinterpret_cast"	{ keyword("reinterpret_cast"); }
"static_cast"		{ keyword("static_cast"); }
"true"				{ keyword("true"); }
"typeid"			{ keyword("typeid"); }
"using"				{ keyword("using"); }
"wchar_t"			{ keyword("wchar_t"); }

[a-zA-Z0-9]			{ skipword(); }

.					{ /* ignore bad characters */ }

%%

/* Does not handle multiple line comments */
static void comment(end)
int end;
{	/* skip to end of comment/line */
#if 0
	extern int Tlrow;
	int p, c;
	int start = yycol - 2, len = 2;
	char *s = yyptr - 2;

	if((p = input()) != 0) ++len;
	while(((c = input()) != '/' || p != '*') && c)
	{
		p = c;
		++len;
	}
	if(c) len += 2;	/* found end delimiter */

	BoldComment(Tlrow, start, s, len);
#else
	int p, c;
	for(p = input(); (c = input()) && (c != '/' || p != '*'); p = c) ;
#endif
}


#if 0
yywrap()
{
	return(1);
}


void keyword(s)
char *s;
{
	extern int Tlrow;
	int c, len;
	
	if(isalnum((c = input())) || c == '_')
		/* not a complete keyword (i.e. integer instead of in) */
		skipword();
	else
	{
		unput(c);
		len = strlen(s);
		BoldWord(Tlrow, yycol - len, s, len);
	}
}


skipword()
{	/* skip word */
	int c;
	
	while((isalnum(c = input()) || c == '_') && c != 0) ;
}
#endif
