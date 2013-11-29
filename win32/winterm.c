#include "../z.h"

int Clrcol[ROWMAX + 1];		/* Clear if past this */

int Prow, Pcol;			/* Point row and column */
static int Srow = -1, Scol = -1;		/* Saved row and column */
int Colmax = 80, Rowmax = 25;	/* Row and column maximums */ // SAM HARDCODE

HANDLE hstdin, hstdout;

/* Come here on SIGHUP or SIGTERM */
void hang_up(int signal)
{
	struct buff *tbuff;

	InPaw = true;	/* Kludge to turn off error */
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next) {
		if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF) && bfname()) {
			bswitchto(tbuff);
			bwritefile(bfname());
		}
#if SHELL
		if (tbuff->child != EOF)
			unvoke(tbuff, false);
#endif
	}
#if SHELL
	checkpipes(0);
#endif
	tfini();
	exit(1);
}

void tinit(void) {
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
} // SAM FIXME

void tfini(void) {} // SAM FIXME

void setmark(bool prntchar)
{
	tstyle(T_REVERSE);
	tprntchar(prntchar ? Buff() : ' ');
	tstyle(T_NORMAL);
}

void termsize(void)
{
	// SAM FIXME
	Rowmax = 24;
	Colmax = 80;
}

static int tabsize(int col)
{	/* optimize for most used tab sizes */
	switch (Tabsize) {
	case 2:
	case 4:
	case 8:
		return Tabsize - (col & (Tabsize - 1));
	default:
		return Tabsize - (col % Tabsize);
	}
}
/* Print a char. */
void tprntchar(Byte ichar)
{
	int tcol;

	if (ZISPRINT(ichar)) {
		tforce();
		tputchar(ichar);
		++Scol;
		++Pcol;
		if (Clrcol[Prow] < Pcol)
			Clrcol[Prow] = Pcol;
	}
	else
		switch (ichar) {
		case '\t':
			if (InPaw)
				tprntstr("^I");
			else
			for (tcol = tabsize(Pcol); tcol > 0; --tcol)
				tprntchar(' ');
			break;

		case 0x89:
			tstyle(T_BOLD);
			tprntstr("~^I");
			tstyle(T_NORMAL);
			break;

		default:
			tstyle(T_BOLD);
			if (ichar & 0x80) {
				tprntchar('~');
				tprntchar(ichar & 0x7f);
			}
			else {
				tprntchar('^');
				tprntchar(ichar ^ '@');
			}
			tstyle(T_NORMAL);
			break;
	}
}

/* Calculate the width of a character.
* The 'adjust' parameter adjusts for the end of line.
*/
int chwidth(Byte ch, int col, bool adjust)
{
	int wid;

	if (ZISPRINT(ch))
		return 1;
	switch (ch) {
	case '\n':
		return InPaw ? 2 : 0;
	case '\t':
		wid = tabsize(col);
		if (col + wid >= tmaxcol())
			wid = tmaxcol() - col + Tabsize - 1;
		if (!adjust)
			wid = MIN(wid, Tabsize);
		return InPaw ? 2 : wid;
	default:
		wid = ((ch & 0x80) && !isprint(ch & 0x7f)) ? 3 : 2;
		if (adjust) {
			int delta = col + wid - tmaxcol();
			if (delta >= 0)
				wid += delta + 1;
		}
		return wid;
	}
}

void tprntstr(const char *str)
{
	while (*str)
		tprntchar(*str++);
}

void t_goto(int row, int col)
{
	tsetpoint(row, col);
	tforce();
}

int prefline(void)
{
	int line, w;

	w = wheight();
	line = PREFLINE * w / (Rowmax - 2);
	return line < w ? line : w >> 1;
}

void tforce(void)
{
	if (Scol != Pcol || Srow != Prow) {
		// WIN32
		COORD where;

		// SAM Save Scol/Srow as COORD?
		where.X = Pcol;
		where.Y = Prow;
		SetConsoleCursorPosition(hstdout, where);
		Srow = Prow;
		Scol = Pcol;
	}
}

void tcleol(void)
{
	if (Pcol < Clrcol[Prow]) {
		tforce();
		// SAM FIXME fputs("\033[K", stdout);
		Clrcol[Prow] = Pcol;
	}
}

void tclrwind(void)
{
	// SAM FIXME
	memset(Clrcol, 0, ROWMAX);
	Prow = Pcol = 0;
}

void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

	switch (cur_style = style) {
	case T_NORMAL:
		SetConsoleTextAttribute(hstdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		break;
	case T_STANDOUT:
	case T_REVERSE:
		SetConsoleTextAttribute(hstdout, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED);
		break;
	case T_BOLD:
	case T_COMMENT:
		break;
	}
	fflush(stdout);
}


void tbell(void) {} // SAM FIXME

int strcasecmp(const char *a, const char *b)
{
	while (*a && *b)
	if (tolower(*a) == tolower(*b)) {
		++a; ++b;
	} else
			break;
	return *a - *b;
}

int strncasecmp(const char *a, const char *b, int n)
{
	while (*a && *b && n > 0)
		if (tolower(*a) == tolower(*b)) {
			++a; ++b; --n;
		} else
			break;
	return n;
}

// SAM we can get mutch smarter with tputchar and tflush...
void tputchar(Byte c)
{
	DWORD written;

	WriteConsole(hstdout, &c, 1, &written, NULL);
}

void tflush(void) {}