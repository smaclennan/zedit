#include "tinit.h"

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
