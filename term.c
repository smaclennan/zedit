/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include "keys.h"
#if TERMINFO
#include <term.h>
#endif

int SGnum = 0;			/* needed by Termcap/Terminfo but... */

#include <signal.h>
#include <sys/wait.h>	/* need for WNOWAIT */

#if LINUX
#include <termios.h>
struct termios Savetty;
struct termios settty;
#elif SYSV2
#include <termio.h>
struct termio Savetty;
struct termio settty;
#elif BSD
#include <sgtty.h>
#if SUNBSD
int gtty ARGS((int, struct sgttyb *));
int stty ARGS((int, struct sgttyb *));
#endif
struct sgttyb Savetty;
struct sgttyb settty;
struct tchars Savechars;
struct tchars setchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
struct ltchars Savelchars;
struct ltchars setlchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#endif

size_t Clrcol[ ROWMAX + 1 ];	/* Clear if past this */

int Prow, Pcol;				/* Point row and column */
int Srow, Scol;				/* Saved row and column */
size_t Colmax, Rowmax;			/* Row and column maximums */
int Tstart;					/* Start column and row */

#if !XWINDOWS
#ifdef SIGWINCH
/* This is called if the window has changed size.
 * If Exitflag is set, we are not ready to update display yet.
 */
void sigwinch(sig)
int sig;
{
	extern Boolean Exitflag;

	if(Exitflag)
		Termsize();
	else
	{
		Zredisplay();		/* update the windows */
		Refresh();			/* force a screen update */
	}

#ifdef SYSV2
	signal(SIGWINCH, sigwinch);
#endif
}
#endif

/* Initalize the terminal. */
void Tinit()
{
	extern Boolean Exitflag;

#if TERMINFO || ANSI
	/* Initialize from the Terminfo database. Do this first - it may exit */
	TIinit();
#endif

	Termsize();

#if LINUX
	tcgetattr( fileno(stdin), &Savetty );
	tcgetattr( fileno(stdin), &settty );
	settty.c_iflag = Vars[VFLOW].val ? (IXON | IXOFF) : 0;
	settty.c_oflag = TAB3;
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	tcsetattr( fileno(stdin), TCSANOW, &settty );
#elif SYSV2
	ioctl( fileno(stdin), TCGETA, &Savetty );
	ioctl( fileno(stdin), TCGETA, &settty );
	settty.c_iflag = Vars[VFLOW].val ? (IXON | IXOFF) : 0;
	settty.c_oflag = TAB3;
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	ioctl( fileno(stdin), TCSETAW, &settty );
#elif BSD
	gtty( fileno(stdin), &Savetty );
	gtty( fileno(stdin), &settty );

	/* set CBREAK (raw) mode no ECHO, leave C-Ms alone so we can read them */
	settty.sg_flags |= CBREAK;
	settty.sg_flags &= ~(ECHO | CRMOD);
	stty( fileno(stdin), &settty );

	ioctl( fileno(stdin), TIOCGETC, &Savechars );
	ioctl( fileno(stdin), TIOCSETC, &setchars );

	ioctl( fileno(stdin), TIOCGLTC, &Savelchars );
	ioctl( fileno(stdin), TIOCSLTC, &setlchars );
#endif

	signal( SIGHUP,  Hangup );
	signal( SIGTERM, Hangup );
#if PIPESH
#if !SYSV4 || !defined(WNOWAIT)
	signal( SIGCLD,  Sigchild );
#endif
	signal( SIGPIPE, Sigchild );
#endif
#if BSD
	signal( SIGTSTP, SIG_DFL );		/* set signals so that we can */
	signal( SIGCONT, Tinit );		/* suspend & restart Zedit */
#endif
#ifdef SIGWINCH
	signal(SIGWINCH, sigwinch);		/* window has changed size - update */
#endif

	if(Rowmax < 3)
	{	/* screen too small */
		Tfini();
		exit(1);
	}

	Srow = Scol = -1;	/* undefined */
	if( Exitflag )
		Initline();		/* Curwdo not defined yet */
	else
		Zredisplay();
}


void Tfini()
{
#if LINUX
  tcsetattr( fileno(stdin), TCSAFLUSH, &Savetty );
#elif SYSV2
	ioctl( fileno(stdin), TCSETAF, &Savetty );
#elif BSD
	stty( fileno(stdin), &Savetty );
	ioctl( fileno(stdin), TIOCSETC, &Savechars );
	ioctl( fileno(stdin), TIOCSLTC, &Savelchars );
#endif

	if( Vars[VCLEAR].val )
	   	Tclrwind();
	else
	{
		Clrecho();
		Tgoto(Rowmax - 1, 0);
	}
	Tflush();
#if TERMINFO || ANSI
	TIfini();
#endif
}

void Tbell()
{
#if TERMINFO || ANSI
	if (Vars[VVISBELL].val) {
#ifdef __linux__
		fputs("\033[?5h", stdout);
		fflush(stdout);
		usleep(100000);
		fputs("\033[?5l", stdout);
#elif TERMINFO
		TPUTS(flash_screen);
#endif
	} else
#endif
		if( Vars[VSILENT].val == 0 )
			putchar( '\7' );
}


/* Actually display the mark */
void Tsetmark(ch, row, col)
Byte ch;
int row, col;
{
	Tstyle(T_REVERSE);
	Tprntchar(ch);
	Tstyle(T_NORMAL);
}

void SetMark(prntchar)
Boolean prntchar;
{
		Tstyle(T_REVERSE);
		Tprntchar(prntchar ? Buff() : ' ');
		Tstyle(T_NORMAL);
}

/*
 * Set Rowmax and Colmax.
 *		1. Use environment variables.
 *		2. Use given variables.
 *		3. Default to 24x80
 *		4. Limit to ROWMAXxCOLMAX.
 */
void Termsize()
{
	extern char *getenv();
	int rows, cols;
#if !HAS_RESIZE	&& !XWINDOWS
	char *n;
#endif

	Tsize(&rows, &cols);

#if !HAS_RESIZE	&& !XWINDOWS
	if(!(n = getenv("LINES")) || (Rowmax = atoi(n)) <= 0)
#endif
		Rowmax = rows <= 0 ? 24 : rows;
	if(Rowmax > ROWMAX) Rowmax = ROWMAX;

#if !HAS_RESIZE && !XWINDOWS
	if(!(n = getenv("COLUMNS")) || (Colmax = atoi(n)) <= 0)
#endif
		Colmax = cols <= 0 ? 80 : cols;
	if(Colmax > COLMAX) Colmax = COLMAX;
}
#endif
/* !XWINDOWS */


void ExtendedLineMarker()
{
	int col;

	for(col = Tgetcol(); col < Tmaxcol() - 1; ++col) Tprntchar(' ');
	Tstyle(T_BOLD);
	Tprntchar('>');
	Tstyle(T_NORMAL);
}

void Tprntchar( Byte ichar )
/* Print a char. */
{
	int tcol;

	if(ISPRINT(ichar))
	{
		Tforce();
		Tputchar(ichar);
		++Scol;
		++Pcol;
		if( Clrcol[Prow] < Pcol ) Clrcol[ Prow ] = Pcol;
	}
	else switch( ichar )
	{
		case '\t':
			if( InPaw )
				Tprntstr( "^I" );
			else
			{	/* optimize for most used tab sizes */
				if(Tabsize == 4 || Tabsize == 8)
					tcol = Tabsize - (Pcol & (Tabsize - 1));
				else
					tcol = Tabsize - (Pcol % Tabsize);
				for( ; tcol > 0; --tcol)
					Tprntchar(' ');
			}
			break;

		case 0x89:
			Tstyle( T_BOLD );
			Tprntstr( "~^I" );
			Tstyle( T_NORMAL );
			break;

		default:
			Tstyle( T_BOLD );
			if( ichar & 0x80 )
			{
				Tprntchar( '~' );
				Tprntchar( ichar & 0x7f );
			}
			else
			{
				Tprntchar( '^' );
				Tprntchar( ichar ^ '@' );
			}
			Tstyle( T_NORMAL );
			break;
	}
}


/* Calculate the width of a character.
 * The 'adjust' parameter adjusts for the end of line.
*/
int Width( Byte ch, int col, Boolean adjust )
{
	int wid;

	if(ISPRINT(ch)) return 1;
	if(InPaw && (ch == '\n' || ch == '\t')) return 2;
	if(ch == '\n') return 0;

#ifdef SAM
	col = col % (Tmaxcol() - 1 - Tstart );
#endif
	if(ch == '\t')
	{
		if(Tabsize == 4 || Tabsize == 8)
			wid = Tabsize - (col & (Tabsize - 1));
		else
			wid = Tabsize - (col % Tabsize);
		if(col + wid >= Tmaxcol())
			wid = Tmaxcol() - col + Tabsize - 1 - Tstart;
		if(!adjust) wid = MIN(wid, Tabsize);
	}
	else
	{
		int delta;

		wid = ((ch & 0x80) && !isprint(ch & 0x7f)) ? 3 : 2;
		if(adjust && (delta = col + wid - Tmaxcol() - Tstart) >= 0)
			wid += delta + 1 - Tstart;
	}
	return wid;
}


void Tprntstr( str )
char *str;
{
	while( *str ) Tprntchar( *str++ );
}


void Tgoto( row, col )
int row, col;
{
	Tsetpoint( row, col );
	Tforce();
}

void Titot( cntr )
unsigned cntr;
/* Print a decimal number. */
{
	if( cntr > 9 ) Titot( cntr / 10 );
	Tprntchar( cntr % 10 + '0' );
}


int Prefline()
{
	int line, w;

	w = Wheight();
	line = PREFLINE * w / (Rowmax - 2);
	return(line < w ? line : w >> 1);
}

#if !XWINDOWS
void Tforce()
{
	if( Scol != Pcol || Srow != Prow )
	{
#if TERMINFO
		TPUTS(tparm(cursor_address, Prow, Pcol));
#else
		printf("\033[%d;%dH", Prow, Pcol);
#endif
		Srow = Prow;
		Scol = Pcol;
	}
}


void Tcleol()
{
	if( Pcol < Clrcol[Prow] )
	{
		Tforce();
#if TERMINFO
		TPUTS(clr_eol);
#else
		TPUTS("\033[K");
#endif
		Clrcol[ Prow ] = Pcol;
	}
}


void Tclrwind()
{
#if TERMINFO
	TPUTS(clear_screen);
#else
	TPUTS("\033[2J");
#endif
	memset( Clrcol, 0, ROWMAX );
	Prow = Pcol = 0;
	Tflush();
}

/* for tputs this must be a function */
#ifdef __STDC__
#if LINUX
int _putchar(int ch)
#else
int _putchar(char ch)
#endif
#else
int _putchar(ch)
char ch;
#endif
{
	putchar(ch);
	return 0;	/*shutup*/
}

#if LINUX
#if 0
void DumpTermios(struct termios *tty)
{
  int i;

  Dbg("Termios:\n");
	Dbg("\tc_iflag	%x\n", tty->c_iflag);
	Dbg("\tc_oflag	%x\n", tty->c_oflag);
	Dbg("\tc_cflag	%x\n", tty->c_cflag);
	Dbg("\tc_lflag	%x\n", tty->c_lflag);
	Dbg("\tc_line	%x\n", tty->c_line);
	Dbg("\tc_ispeed	%x\n", tty->c_ispeed);
	Dbg("\tc_ospeed	%x\n", tty->c_ospeed);
  for(i = 0; i < NCCS; ++i)
    Dbg("\tc_cc[%2d]  %x\n", i, tty->c_cc[i]);
}
#endif
#endif

void Newtitle(str) char *str; {}
#endif /* !XWINDOWS */
