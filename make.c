/* make.c - Zedit make commands
 * Copyright (C) 1988-2013 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "z.h"

#if SHELL
/* This is cleared in Zmake and set in Znexterror.
 * If clear, the make buffer is scrolled up. Once a next error is
 * called, the buffer is kept at the error line.
 */
static int NexterrorCalled;

static int parse(char *fname);

static int set_cmd(int which, char *prompt)
{
	char cmd[STRMAX + 1];

	Argp = false;
	strcpy(cmd, VARSTR(which));
	if (getarg(prompt, cmd, STRMAX))
		return 0;
	if (VARSTR(which))
		free(VARSTR(which));
	VARSTR(which) = strdup(cmd);
	return 1;
}

/* Do a "make" command - basically a shell command in the ".make" buffer */
void Zmake(void)
{
	struct buff *mbuff;

	if (!VARSTR(VMAKE))
		VARSTR(VMAKE) = strdup(MAKE_CMD);

	NexterrorCalled = 0;	/* reset it */
	Arg = 0;
	if (Argp)
		if (!set_cmd(VMAKE, "Make: "))
			return;
	saveall(true);
#ifdef PIPESH
	mbuff = cfindbuff(MAKEBUFF);
	if (mbuff && mbuff->child != EOF) {
		putpaw("Killing current make.");
		unvoke(mbuff, true);
		clrecho();
	}
#endif
	mbuff = cmdtobuff(MAKEBUFF, VARSTR(VMAKE));
	if (mbuff)
		message(mbuff, VARSTR(VMAKE));
	else
		error("Unable to execute make.");
}

/* Do a "make" command - basically a shell command in the ".make" buffer */
void Zgrep(void)
{
	struct buff *mbuff;
	char cmd[STRMAX * 3], input[STRMAX + 1], files[STRMAX + 1];

	if (!VARSTR(VGREP))
		VARSTR(VGREP) = strdup(GREP_CMD);

	NexterrorCalled = 0;	/* reset it */
	Arg = 0;
	if (Argp)
		if (!set_cmd(VGREP, "grep: "))
			return;

	getbword(input, STRMAX, bistoken);
	if (getarg("Regex: ", input, STRMAX))
		return;

	if (Curbuff->bmode & CMODE)
		strcpy(files, "*.[ch]");
	else if (Curbuff->bmode & SHMODE)
		strcpy(files, "*.sh");
	else
		strcpy(files, "*");
	if (getarg("File(s): ", files, STRMAX))
		return;

	snprintf(cmd, sizeof(cmd), "sh -c '%s \"%s\" %s'",
		 VARSTR(VGREP), input, files);

	saveall(true);
#ifdef PIPESH
	mbuff = cfindbuff(MAKEBUFF);
	if (mbuff && mbuff->child != EOF) {
		error("Make buffer in use...");
		return;
	}
#endif
	mbuff = cmdtobuff(MAKEBUFF, cmd);
	if (mbuff)
		message(mbuff, cmd);
	else
		error("Unable to execute grep.");
}

void Znexterr(void)
{
	struct wdo *wdo;
	struct buff *save, *mbuff;
	char fname[STRMAX + 1];
	char path[PATHMAX + 1];
	int line;

	mbuff = cfindbuff(MAKEBUFF);
	if (!mbuff) {
		tbell();
		return;
	}
	save = Curbuff;
	bswitchto(mbuff);
	if (!NexterrorCalled) {
		NexterrorCalled = 1;
		btostart();
	}
	line = parse(fname);
	if (line) {
		vsetmrk(Curbuff->mark);
		bmrktopnt(Curbuff->mark);
		tobegline();
		bswappnt(Curbuff->mark);
		vsetmrk(Curbuff->mark);
		wdo = findwdo(mbuff);
		if (wdo)
			mrktomrk(wdo->wstart, Curbuff->mark);
		pathfixup(path, fname);
		findfile(path);
		Argp = true;
		Arg = line;
		Zlgoto();
		tobegline();
	} else {
		btoend();
		bmrktopnt(Curbuff->mark);
		bswitchto(save);
		putpaw("No more errors");
	}
	Argp = false;
	Arg = 0;
}

#ifdef PIPESH
/* kill the make */
void Zkill(void)
{
	unvoke(cfindbuff(MAKEBUFF), false);
}
#else
void Zkill(void) { tbell(); }
#endif

/* Check if it is a warning or an error.
 * Currently works for GCC, g++, MIPs.
 */
static bool IsWarning;

static int isnotws(void)
{
	return Buff() != '\n' && Buff() != '\t' && Buff() != ' ';
}

static bool warning(void)
{
	if (Argp) {
		if (IsWarning)
			return true;

		if (Buff() == ':') {
			char word[10], *p;
			getbword(p = word, 10, isnotws);
			if (*p == ':')
				++p;
			return strcmp(p, "warning:") == 0;
		}
	}
	return 0;
}

/* Find the next error in the .make buffer.
 * Ignores lines that start with a white space.
 * Supported:
 *	CC		"<fname>", line <line>: <msg>
 *	GNU C		<fname>:<line>: (error: | warning:) <msg>
 *      G++		<fname>:<line>: [warning:] <msg>
 *	HP CC		cc: "<fname>", line <line>: <msg>
 *	AS		as: "<fname>", line <line>: <msg>
 *	High C		[Ew] "<fname>",L<line>/C<column>: <msg>
 *	Microsoft	<fname>(<line>) : <msg>
 *	MIPS C		cfe: (Warning|error): <fname>, <line>: <msg>
 *	MIPS AS		as(0|1): (Warning|error): <fname>, line <line>: <msg>
 *
 *	ignores		conflicts: <line>
 */
static int parse(char *fname)
{
	char word[41], *p;
	int line, n;

	while (!bisend()) {
		IsWarning = 0;

		/* get first word in line */
		n = getbword(word, 40, isnotws);

		/* check for: as: cc: */
		if (strcmp(word, "as:") == 0 || strcmp(word, "cc:") == 0)
			bmove(4);
		/* check for cfe:/as0:/as1: (MIPS) */
		else if (strcmp(word, "cfe:") == 0 ||
			 strcmp(word, "as0:") == 0 ||
			 strcmp(word, "as1:") == 0) {
			bmove(5);
			IsWarning = Buff() == 'W';
			bmove(IsWarning ? 9 : 7);
		}
		/* check High C for "E " or "w " */
		else if (n == 1 && (*word == 'E' || *word == 'w'))
			bmove(2);
		else if (strcmp(word, "conflicts:") == 0) {
			bcsearch(NL);	/* skip line */
			continue;
		}
		/* try to get the fname */
		if (Buff() == '"')
			bmove1();
		for (p = fname; !strchr("\",:(\n", Buff()); bmove1())
			*p++ = Buff();
		*p = '\0';
		if (Buff() == '"')
			bmove1();

		/* try to get the line */
		if (Buff() == ':' || Buff() == '(')
			bmove1();
		else if (Buff() == ',') {
			while (!isdigit(Buff()) && Buff() != '\n' && !bisend())
				bmove1();
		}

		/* look for line number */
		line = batoi();
		if (line != 0 && !warning())
			return line;

		/* skip to next line */
		bcsearch(NL);
	}
	return 0;
}
#else
void Zmake(void) { tbell(); }
void Znexterr(void) { tbell(); }
void Zkill(void) { tbell(); }
void Zgrep(void) { tbell(); }
#endif /* SHELL */
