/* Copyright (C) 1988-2017 Sean MacLennan */

#include "tinit.h"
#include "keys.h"
#include <poll.h>
#include <signal.h>
#include <unistd.h>

/** @addtogroup term
 * @{
 */

/** The special multi-byte keys. This array may be updated dynamically
 * if we detect ESC O keys.
 * Note: We can currently only have 32 specials.
 */
static char *Tkeys[] = {
	"\033[A",	/* up */
	"\033[B",	/* down */
	"\033[C",	/* right */
	"\033[D",	/* left */

	"\033[2~",	/* insert */
	"\033[3~",	/* delete */
	"\033[5~",	/* page up */
	"\033[6~",	/* page down */
	"\033[7~",	/* home */
	"\033[8~",	/* end */

	"\033[11~",	/* f1 */
	"\033[12~",	/* f2 */
	"\033[13~",	/* f3 */
	"\033[14~",	/* f4 */
	"\033[15~",	/* f5 */
	"\033[17~",	/* f6 */
	"\033[18~",	/* f7 */
	"\033[19~",	/* f8 */
	"\033[20~",	/* f9 */
	"\033[21~",	/* f10 */
	"\033[23~",	/* f11 */
	"\033[24~",	/* f12 */

	"\033[7^",	/* C-home */
	"\033[8^"	/* C-end */
};

_Static_assert((sizeof(Tkeys) / sizeof(char *)) == NUM_SPECIAL,
	       "Tkeys != NUM_SPECIAL");
/* Currently NUM_SPECIAL must fit in 32 bits */
_Static_assert(NUM_SPECIAL <= 32, "Too many NUM_SPECIAL");

void st_hack(void)
{
	Tkeys[4] = "\033[4h"; /* insert */
	Tkeys[5] = "\033[P";  /* delete */
	Tkeys[8] = "\033[H";  /* home */
	Tkeys[9] = "\033[4~"; /* end */

	/* C-home == home */
	Tkeys[23] = "\033[J"; /* C-end */
}

#ifdef __QNXNTO__
#define KEY_MASK2		0x00fffff0

static char *Tkeys2[] = {
	"",	/* up */
	"",	/* down */
	"",	/* right */
	"",	/* left */

	"\033[@",	/* insert */
	"\033[P",	/* delete */
	"\033[V",	/* page up */
	"\033[U",	/* page down */
	"\033[H",	/* home */
	"\033[Y",	/* end */

	"\033OP",	/* f1 */
	"\033OQ",	/* f2 */
	"\033OR",	/* f3 */
	"\033OS",	/* f4 */
	"\033OT",	/* f5 */
	"\033OU",	/* f6 */
	"\033Op",	/* f7 */
	"\033Oq",	/* f8 */
	"\033Or",	/* f9 */
	"\033Os",	/* f10 */
	"\033Ot",	/* f11 */
	"\033Ou",	/* f12 */

	"\033[h",	/* C-home */
	"\033[y"	/* C-end */
};
#endif

/* \cond skip */
/** The size of the keyboard input stack. Must be a power of 2 */
#define CSTACK 16
static Byte cstack[CSTACK]; /**< The keyboard input stack */
static int cptr = -1; /**< Current pointer in keyboard input stack. */
int cpushed; /**< Number of bytes pushed on the keyboard input stack. */
static int Pending; /**< Set to 1 if poll stdin detected input. */

/** This is the lowest level keyboard routine. It reads the keys into
 * a stack then returns the keys one at a time. When the stack is
 * consumed it reads again.
 *
 * The read can block.
 */
static Byte _tgetkb(void)
{
	cptr = (cptr + 1) & (CSTACK - 1);
	if (cpushed)
		--cpushed;
	else {
		Byte buff[CSTACK];
		int i, p = cptr;

		cpushed = read(0, (char *)buff, CSTACK) - 1;
		if (cpushed < 0)
			kill(getpid(), SIGHUP); /* we lost connection */
		for (i = 0; i <= cpushed; ++i) {
			cstack[p] = buff[i];
			p = (p + 1) & (CSTACK - 1);
		}
	}
	return cstack[cptr];
}

/** Push back n keys */
static void tungetkb(int n)
{
	cptr = (cptr - n) & (CSTACK - 1);
	cpushed += n;
}

/** Peek the key at a given offset */
static Byte tpeek(int offset)
{
	return cstack[(cptr + offset) & (CSTACK - 1)];
}

/** Check if the keyboard input is "special", i.e. One of the
 * multi-byte #Tkeys.
 */
static int check_specials(void)
{
	int i, j, bit, mask = KEY_MASK;
#ifdef KEY_MASK2
	int mask2 = KEY_MASK2;
#else
	int mask2 = 0;
#endif

	for (j = 1; mask || mask2; ++j) {
		int cmd = _tgetkb() & 0x7f;

		if (mask) {
			for (bit = 1, i = 0; i < NUM_SPECIAL; ++i, bit <<= 1)
				if ((mask & bit) && cmd == Tkeys[i][j]) {
					if (Tkeys[i][j + 1] == '\0')
						return i + SPECIAL_START;
				} else
					mask &= ~bit;
		}
#ifdef KEY_MASK2
		if (mask2) {
			for (bit = 1, i = 0; i < NUM_SPECIAL; ++i, bit <<= 1)
				if ((mask2 & bit) && cmd == Tkeys2[i][j]) {
					if (Tkeys2[i][j + 1] == '\0')
						return i + SPECIAL_START;
				} else
					mask2 &= ~bit;
		}
#endif
	}

	/* No match - push back the chars */
	tungetkb(j);

	if (tpeek(2) == 'O')
		/* Check for ESC O keys - happens a lot on ssh */
		switch (tpeek(3)) {
		case 'A'...'D':
			/* rewrite the arrow keys */
			Tkeys[0] = "\033OA";	/* up */
			Tkeys[1] = "\033OB";	/* down */
			Tkeys[2] = "\033OC";	/* right */
			Tkeys[3] = "\033OD";	/* left */
			/* skip the ESC */
			_tgetkb();
			return check_specials();
		case 'P'...'S':
			/* rewrite F1 to F4 */
			Tkeys[10] = "\033OP";	/* up */
			Tkeys[11] = "\033OQ";	/* down */
			Tkeys[12] = "\033OR";	/* right */
			Tkeys[13] = "\033OS";	/* left */
			/* skip the ESC */
			_tgetkb();
			return check_specials();
		}

	return _tgetkb() & 0x7f;
}
/* \endcond */

/** Get keyboard input. Handles the special keys.
 * @return The next key input.
 */
int tgetkb(void)
{
	int cmd = _tgetkb() & 0x7f;

	Pending = 0;

	/* All special keys start with ESC */
	if (cmd == '\033' && tkbrdy())
		return check_specials();

	return cmd;
}

/* \cond skip */
/** A static pollfd for stdin. */
static struct pollfd stdin_fd = { .fd = 0, .events = POLLIN };
/* \endcond */

/** Is there a key waiting? Non-blocking command.
 * @return 1 if a key is pending.
 */
int tkbrdy(void)
{
	if (cpushed || Pending)
		return 1;

	return Pending = poll(&stdin_fd, 1, 0) == 1;
}

/** Delay for a set time or until there is keyboard input.
 * @param ms The delay in milliseconds.
 * @return 1 if we delayed, 0 if a key is pending.
 */
int tdelay(int ms)
{
	if (cpushed || Pending)
		return 0;

	return poll(&stdin_fd, 1, ms) != 1;
}

#if defined(TERMCAP) || defined(TERMINFO)
void set_tkey(int i, char *key)
{
	if (*key == '\e')
		Tkeys[i] = key;
}
#endif
/* @} */
