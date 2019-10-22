#include "tinit.h"

extern HANDLE hstdout;

#define ATTR_NORMAL	(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY)
#define ATTR_REVERSE	(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY)

static void tforce(void)
{
	if (Scol != Pcol || Srow != Prow) {
		COORD where;

		where.X = Pcol;
		where.Y = Prow;
		SetConsoleCursorPosition(hstdout, where);
		Srow = Prow;
		Scol = Pcol;
	}
}

void tputchar(Byte ch)
{
	tforce();

	DWORD written;
	WriteConsole(hstdout, &ch, 1, &written, NULL);

	++Scol;
	++Pcol;
	if (Clrcol[Prow] < Pcol)
		Clrcol[Prow] = Pcol;
}

void t_goto(int row, int col)
{
	Prow = row;
	Pcol = col;
	tforce();
}

void tcleol(void)
{
	if (Prow >= ROWMAX)
		Prow = ROWMAX - 1;

	if (Pcol < Clrcol[Prow]) {
		COORD where;
		DWORD written;

		where.X = Pcol;
		where.Y = Prow;
		FillConsoleOutputCharacter(hstdout, ' ', Clrcol[Prow] - Pcol,
					   where, &written);

		/* This is to clear a possible mark */
		if (Clrcol[Prow])
			where.X = Clrcol[Prow] - 1;
		FillConsoleOutputAttribute(hstdout, ATTR_NORMAL, 1,
					   where, &written);

		Clrcol[Prow] = Pcol;
	}
}

void tclrwind(void)
{
	COORD where = { 0 };
	DWORD written;
	FillConsoleOutputAttribute(hstdout, ATTR_NORMAL, COLMAX * ROWMAX,
				   where, &written);
	FillConsoleOutputCharacter(hstdout, ' ', COLMAX * ROWMAX,
				   where, &written);

	memset(Clrcol, 0, sizeof(Clrcol));
	Prow = Pcol = 0;
	tflush();
}

void tsize(int *rows, int *cols)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD size;

	*cols = 80;
	*rows = 30;

	if (GetConsoleScreenBufferInfo(hstdout, &info)) {
		size.Y = info.srWindow.Bottom - info.srWindow.Top + 1;
		size.X = info.srWindow.Right - info.srWindow.Left + 1;
	}

	if (size.X != *cols || size.Y != *rows) {
		if (size.X > COLMAX) size.X = COLMAX;
		if (size.Y > ROWMAX) size.Y = ROWMAX;
		*cols = size.X;
		*rows = size.Y;
		SetConsoleScreenBufferSize(hstdout, size);
	}
}

void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

	switch (style) {
	case T_NORMAL:
		SetConsoleTextAttribute(hstdout, ATTR_NORMAL);
		break;
	case T_REVERSE:
		SetConsoleTextAttribute(hstdout, ATTR_REVERSE);
		break;
	case T_BOLD:
		SetConsoleTextAttribute(hstdout,
					ATTR_NORMAL | FOREGROUND_INTENSITY);
		break;
	case T_FG + T_BLACK:
		SetConsoleTextAttribute(hstdout, ATTR_NORMAL); break;
	case T_FG + T_RED:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED); break;
	case T_FG + T_GREEN:
		SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN); break;
	case T_FG + T_YELLOW:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_GREEN); break;
	case T_FG + T_BLUE:
		SetConsoleTextAttribute(hstdout, FOREGROUND_BLUE); break;
	case T_FG + T_MAGENTA:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_BLUE); break;
	case T_FG + T_CYAN:
		SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN | FOREGROUND_BLUE); break;
	case T_FG + T_WHITE:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	case T_BG + T_BLACK:
		SetConsoleTextAttribute(hstdout, ATTR_REVERSE); break;
	case T_BG + T_RED:
		SetConsoleTextAttribute(hstdout, BACKGROUND_RED); break;
	case T_BG + T_GREEN:
		SetConsoleTextAttribute(hstdout, BACKGROUND_GREEN); break;
	case T_BG + T_YELLOW:
		SetConsoleTextAttribute(hstdout, BACKGROUND_RED | BACKGROUND_GREEN); break;
	case T_BG + T_BLUE:
		SetConsoleTextAttribute(hstdout, BACKGROUND_BLUE); break;
	case T_BG + T_MAGENTA:
		SetConsoleTextAttribute(hstdout, BACKGROUND_RED | BACKGROUND_BLUE); break;
	case T_BG + T_CYAN:
		SetConsoleTextAttribute(hstdout, BACKGROUND_GREEN | BACKGROUND_BLUE); break;
	case T_BG + T_WHITE:
		SetConsoleTextAttribute(hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
		break;
	}

	cur_style = style;
	tflush();
}
