/* shell.c - shell commands and routines
 * Copyright (C) 1988-2010 Sean MacLennan
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
#ifdef BSD
#include <signal.h>
#endif

static char Command[STRMAX + 1];

static void printexit(int code);
static int pipetobuff(struct buff *buff, char *instr);

static char *get_shell(void)
{
	static char *sh;

	if (!sh) {
		sh = getenv("SHELL");
		if (!sh)
			sh = "/bin/sh";
	}

	return sh;
}

/* Do one shell command to the screen */
#if !defined(BSD)
void Zcmd(void)
{
	char tb[STRMAX * 2];

	Arg = 0;
	if (getarg("! ", Command, STRMAX) == 0) {
		tfini();
		sprintf(tb, "%s -c \"%s\"", get_shell(), Command);
		if (system(tb) == EOF)
			echo("command failed");
		else {
			fputs("\n[Hit Return to continue]", stdout);
			tgetcmd();
			putchar('\n');
		}
		tinit();
	}
}
#endif

/* Do one shell command to the .shell buffer */
void Zcmdtobuff(void)
{
#ifdef PIPESH
	struct wdo *save;
	int rc;

	Arg = 0;
	if (getarg("@ ", Command, STRMAX) == 0) {
		save = Curwdo;
		if (wuseother(SHELLBUFF)) {
			echo("Please wait...");
			rc = pipetobuff(Curbuff, Command);
			if (rc == 0) {
				message(Curbuff, Command);
				btostart();
			}
			Curbuff->bmodf = FALSE;
			printexit(rc);
			wswitchto(save);
		}
	}
#else
	Arg = 0;
	if (getarg("@ ", Command, STRMAX) == 0)
		cmdtobuff(SHELLBUFF, Command);
#endif
}

#ifdef BSD
#ifdef PIPESH
void Zcmd(void)
#else
void Zshell()	/*for tags*/
#endif
{
	tfini();
	kill(getpid(), SIGTSTP);
}
#endif

#ifdef PIPESH
void Zshell(void)
{
	char bname[BUFNAMMAX + 1];
	int i = 0;

	/* create a unique buffer name */
	if (cfindbuff(strcpy(bname, SHELLBUFF)))
		do
			sprintf(bname, "%s%d", SHELLBUFF, ++i);
		while (cfindbuff(bname));

	if (!wuseother(bname) || !doshell())
		tbell();
}

Boolean doshell(void)
{
	char *argv[3];

	argv[0] = get_shell();
	argv[1] = "-i";
	argv[2] = NULL;
	return invoke(Curbuff, argv);
}
#else
static void syerr(int err)
{
	switch (err) {
	case E2BIG:
	case ENOMEM:
		error("Not enough memory");
		break;

	case ENOENT:
		error("Command not found");
		break;

	default:
		error("Unable to execute");
	}
}

void Zshell(void)	/*for tags*/
{
	int err = EOF;

	Arg = 0;
	tfini();
	if (system(get_shell()) == EOF)
		err = errno;
	tinit();
	if (err != EOF)
		syerr(err);
}
#endif

#ifdef PIPESH
struct buff *cmdtobuff(char *bname, char *cmd)
{
	struct buff *tbuff = NULL;
	struct wdo *save;

	save = Curwdo;
	if (wuseother(bname)) {
		if (dopipe(Curbuff, cmd))
			tbuff = Curbuff;
		wswitchto(save);
	}
	return tbuff;
}
#else
struct buff *cmdtobuff(char *bname, char *cmd)
{
	char fname[20];
	int err, one;
	struct buff *sbuff, *tbuff;

	Arg = Argp = 0;
	echo("Working...");
	mktemp(strcpy(fname, ZSHFILE));
	err = dopipe(fname, cmd);
	if (err)
		syerr(err);
	else {
		wuseother(bname);
		breadfile(fname);
		unlink(fname);
		Curbuff->bmodf = FALSE;
		clrecho();
	}
	return err ? NULL : tbuff;
}
#endif

#ifndef PIPESH
int dopipe(char *fname, char *cmd)
{
	char command[STRMAX + 1];

	sprintf(command, "%s >%s 2>&1", cmd, fname);
	if (system(command) == EOF)
		return errno;
	return 0;
}
#endif

/* Returns -1 if popen failed, else exit code.
 * Leaves Point at end of new text.
 */
static int pipetobuff(struct buff *buff, char *instr)
{
	FILE *pfp;
	int c;
	char *cmd = malloc(strlen(instr) + 10);
	if (cmd == NULL)
		return -1;
	sprintf(cmd, "%s 2>&1", instr);
	pfp = popen(cmd, "r");
	if (pfp == NULL)
		return -1;
	while ((c = getc(pfp)) != EOF)
		binsert((char)c);
	free(cmd);
	return pclose(pfp) >> 8;
}

static void printexit(int code)
{
	if (code == 0)
		echo("Done.");
	else if (code == -1)
		echo("Unable to execute.");
	else
		putpaw("Exit %d.", code);
}
#else
void Zshell(void) { tbell(); }
void Zcmd(void) { tbell(); }
void Zcmdtobuff(void) { tbell(); }
#endif /* SHELL */
