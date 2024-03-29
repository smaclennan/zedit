/* Copyright (C) 2017 Sean MacLennan */

#include "buff.h"
#include "keys.h"
#include "winkeys.h"

HANDLE hstdin;
HANDLE hstdout;

/* stack and vars for t[un]getkb / tkbrdy */
#define CSTACK 16 /* must be power of 2 */
static int cstack[CSTACK];
static int cptr = -1;
int cpushed;
static int Pending;

static int dummy_cb(INPUT_RECORD *event) { return 0; }

/* Return non-zero if you don't want tgetkb to handle the event */
int (*winkbd_event_cb)(INPUT_RECORD *event) = dummy_cb;

static short convertKey(KEY_EVENT_RECORD *event)
{
	Byte key = virt[event->wVirtualKeyCode];
	if (event->dwControlKeyState == 0 || key == 0)
		return key;

	if (key >= 128)
		return key;

	if (event->dwControlKeyState & (CAPSLOCK_ON | SHIFT_PRESSED)) {
		switch (key) {
		case '`': key = '~'; break;
		case '1': key = '!'; break;
		case '2': key = '@'; break;
		case '3': key = '#'; break;
		case '4': key = '$'; break;
		case '5': key = '%'; break;
		case '6': key = '^'; break;
		case '7': key = '&'; break;
		case '8': key = '*'; break;
		case '9': key = '('; break;
		case '0': key = ')'; break;
		case '-': key = '_'; break;
		case '=': key = '+'; break;

		case '[': key = '{'; break;
		case ']': key = '}'; break;
		case '\\': key = '|'; break;

		case ';': key = ':'; break;
		case '\'': key = '"'; break;

		case ',': key = '<'; break;
		case '.': key = '>'; break;
		case '/': key = '?'; break;

		default:
			if (isalpha(key))
				key = toupper(key);
		}
	}

	if (event->dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
		return M(toupper(key));

	if (event->dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
		switch (key) {
		case ' ':
		case '\\':
			return 28;
		case ']':
			return 29;
		case '^':
			return 30;
		case '_':
		case '-':
			return 31;
		default:
			if (isalpha(key))
				return key - 'a' + 1;
			else
				return 0;
		}

	return key;
}

void tlinit(void)
{
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(hstdin, WINKBD_EVENT_MASK);
}

int tgetkb(void)
{
	Pending = 0;

	cptr = (cptr + 1) & (CSTACK - 1);
	if (cpushed) {
		--cpushed;
		return cstack[cptr];
	}

	INPUT_RECORD input[CSTACK];
	DWORD i, n;
	int p = cptr;

again:
	if (!ReadConsoleInput(hstdin, input, CSTACK, &n))
		abort(); /* you can install an abort handler */

	for (i = 0; i < n; ++i) {
		if (winkbd_event_cb(&input[i]))
			continue;
		if (input[i].EventType == KEY_EVENT)
			if (input[i].Event.KeyEvent.bKeyDown == 0) {
				cstack[p] = convertKey(&input[i].Event.KeyEvent);
				if (cstack[p]) {
					p = (p + 1) & (CSTACK - 1);
					++cpushed;
				}
			}
	}

	if (cpushed == 0)
		goto again; /* keep reading till we get a key */

	--cpushed;
	return cstack[cptr];
}

int tkbrdy(void)
{
	if (cpushed || Pending)
		return 1;

	return Pending = WaitForSingleObject(hstdin, 0) == WAIT_OBJECT_0;
}

int tdelay(int ms)
{
	if (cpushed || Pending)
		return 0;

	return WaitForSingleObject(hstdin, ms) != WAIT_OBJECT_0;
}
