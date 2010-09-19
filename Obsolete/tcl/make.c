#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <tk.h>

typedef int Boolean;
typedef struct { char *m; } Mark;

#define TRUE		1
#define FALSE		0
#define FORWARD		1
#define BACKWARD	0
#define NL			'\n'

static int Argp;		/* if Argp then do not display warnings */
static char *Buffer;	/* pretend we have a buffer */
static char *Point;		/* current position in buffer */

/* Called from the tcl script */
int ParseMakeLine(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	char fname[256];
	int line;
	
	if(argc != 3)
	{
		interp->result = "Usage: Parse warnings line";
		return TCL_ERROR;
	}

	/* argc[2] will be "1" if warnings wanted */
	Argp = *argv[1] == '0';
	Buffer = argv[2];
	Point = Buffer;		/* start of buffer */
	if((line = Parse(fname)) == 0)
		interp->result = "0";
	else
	{
		static char cmd[256];
		
		sprintf(cmd, "%d:%s", line, fname);
		interp->result = cmd;
	}
	
	return TCL_OK;
}

/*-------------------------- Buffer routines -------------------------*/

#define Buff()			(*Point)
#define Bisend()		(*Point == '\0')
#define Bisstart()		(Point == Buffer)
#define Bmove1()		(Bisend() ? Point : ++Point)
#define Bmrktopnt(mrk)	((mrk)->m = Point)
#define Bpnttomrk(mrk)	(Point = (mrk)->m)


void Bmove(int dist)
{
	if(dist > 0)
		while(dist-- > 0 && !Bisend())   ++Point;
	else
		while(dist++ < 0 && !Bisstart()) --Point;
		
}

Boolean Bcsearch(char c)
{	/* NL should just goto end of buffer */
	if(c != NL)
	{
		char *p;
	
		if((p = strchr(Point, c)) != 0)
		{
			Point = p;
			return 1;
		}
	}
	Point = Buffer + strlen(Buffer);
	return 0;
}

int Batoi()
{
	return (int)strtol(Point, &Point, 0);
}

/*-------------------- Unmodified --------------------*/

int Istoken()
{
	return( isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
			Buff() == '$' || Buff() == '/' );
}


int Isnotws()
{
	return( Buff() != '\n' && Buff() != '\t' && Buff() != ' ' );
}

/* Go forward or back past a thingy */
void Movepast( pred, forward )
int (*pred)();
Boolean forward;
{
	if( !forward ) Bmove( -1 );
	while( !(forward ? Bisend() : Bisstart()) && (*pred)() )
		Bmove( forward ? 1 : -1 );
	if( !forward && !(*pred)() ) Bmove1();
}


/* Go forward or back to a thingy */
void Moveto( pred, forward )
int (*pred)();
Boolean forward;
{
	if( !forward ) Bmove( -1 );
	while( !(forward ? Bisend() : Bisstart()) && !(*pred)() )
		Bmove( forward ? 1 : -1 );
	if( !forward && !Bisstart() ) Bmove1();
}

/*
	Get the word at the current buffer point and store in 'word'.
	Get at the most 'max' characters.
	Leaves the point alone.
*/
Boolean Getbword( word, max, valid )
char word[];
int max;
int (*valid)();
{
	int i;
	Mark tmark;
	
	Bmrktopnt( &tmark );
	Moveto( Istoken, FORWARD );
	if( Bisend() )
		Moveto( Istoken, BACKWARD );
	Movepast( Istoken, BACKWARD );
	for( i = 0; !Bisend() && valid() && i < max; ++i, Bmove1() )
		word[ i ] = Buff();
	word[ i ] = '\0';
	Bpnttomrk( &tmark );
	return( i );
}


/* Check if it is a warning or an error.
 * Currently works for GCC, g++, MIPs.
 */
static Boolean IsWarning = 0;

static Boolean Warning()
{
	if(Argp)
	{
		if(IsWarning) return TRUE;

		if(Buff() == ':')
		{
			char word[10], *p;
			Getbword(p = word, 10, Isnotws);
			if(*p == ':') ++p;
			return strcmp(p, "warning:") == 0;
		}
	}
	return 0;
}

/* Find the next error in the .make buffer.
 * Returns 0 if no errors found, else returns line number of error
 *		and puts the file name in the argument.
 *
 * Ignores lines that start with a white space.
 * Supported:
 *	CC 			"<fname>", line <line>: <msg>
 *	GNU C 		<fname>:<line>: (error: | warning:) <msg>
 *  G++			<fname>:<line>: [warning:] <msg>
 *	HP CC		cc: "<fname>", line <line>: <msg>
 *	AS 			as: "<fname>", line <line>: <msg>
 *	High C		[Ew] "<fname>",L<line>/C<column>: <msg>
 *	Microsoft	<fname>(<line>) : <msg>
 *	MIPS C		cfe: (Warning|Error): <fname>, <line>: <msg>
 *	MIPS AS		as(0|1): (Warning|Error): <fname>, line <line>: <msg>
 *	Turbo C		(Error|Warning) <fname> <line>: <msg>	MSDOS only
 *	
 *	ignores		conflicts: <line>
 */
int Parse( fname )
char *fname;
{
	char word[41], *p;
	int line, n;

	while( !Bisend() )
	{
		IsWarning = 0;

		/* get first word in line */
		n = Getbword(word, 40, Isnotws);
		/* check for: as: cc: */
		if(strcmp(word, "as:") == 0 || strcmp(word, "cc:") == 0)
			Bmove(4);
		/* check for cfe:/as0:/as1: (MIPS) */
		else if(strcmp(word, "cfe:") == 0 || strcmp(word, "as0:") == 0 ||
			strcmp(word, "as1:") == 0)
		{
			Bmove(5);
			IsWarning = Buff() == 'W';
			Bmove(IsWarning ? 9 : 7);
		}
		/* check High C for "E " or "w " */
		else if(n == 1 && (*word == 'E' || *word == 'w'))
			Bmove(2);
		else if(strcmp(word, "conflicts:") == 0)
		{
			Bcsearch(NL);	/* skip line */
			continue;
		}
#if MSDOS
		/* Turbo C */
		else if(strcmp(word, "Error") == 0 || strcmp(word, "Warning") == 0)
			Bmove(n + 1);
#endif

		/* try to get the fname */
		if(Buff() == '"') Bmove1();
		for(p = fname; !strchr("\",:( \n", Buff()); Bmove1())
			*p++ = Buff();
		*p = '\0';
		if(Buff() == '"') Bmove1();

		/* try to get the line */
		if( Buff() == ':' || Buff() == '(' )
			Bmove1();
		else if(Buff() == ',')
		{
			while(!isdigit(Buff()) && Buff() != '\n' && !Bisend())
				Bmove1();
		}
		if((line = Batoi()) != 0 && !Warning())
			return line;

		/* skip to next line */
		Bcsearch(NL);
	}
	return 0;
}
