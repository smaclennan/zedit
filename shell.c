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

#ifdef SHELL
#ifdef BSD
#include <signal.h>
#endif

static void printexit(int code);
static int bufftopipe(struct buff *buff, char *cmd);
static int pipetobuff(struct buff *buff, char *instr);

/* Do one shell command to the screen */
#ifdef XWINDOWS
void Zcmd() { tbell(); }	/* no screen */
#elif !defined(BSD)
void Zcmd(void)
{
	char tb[STRMAX * 2];

	Arg = 0;
	if (getarg("! ", Command, STRMAX) == 0) {
		tfini();
		sprintf(tb, "%s -c \"%s\"", Shell, Command);
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

/* Perform man command on pipe, wait for completion, and format */
void Zman(void)
{
	char entry[STRMAX + 5], *p;
	int rc;
	struct wdo *save;

	strcpy(entry, "man ");
	p = entry + strlen(entry);
	getbword(p, STRMAX, bistoken);	/* get word */
	if (getarg("Man: ", p, STRMAX))
		return;

	save = Curwdo;
	if (wuseother(MANBUFF)) {
		echo("Please wait...");
		rc = pipetobuff(Curbuff, entry);
		if (rc == 0) {
			/* remove the underlines */
			message(Curbuff, p);
			btoend();
			while (bcrsearch('\010')) {
				bmove(-1);
				bdelete(2);
			}
		}
		Curbuff->bmodf = FALSE;
		Curbuff->bmode |= VIEW;
		printexit(rc);
		wswitchto(save);
	}
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

	argv[0] = Shell;
	argv[1] = "-i";
	argv[2] = NULL;
	return invoke(Curbuff, argv);
}
#elif defined(SYSV2)
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
	if (system(Shell) == EOF)
		err = errno;
	tinit();
	if (err != EOF)
		syerr(err);
}
#endif

/* send the current buffer as a mail buffer */
void Zmail(void)
{
	char to[STRMAX + 1], subject[STRMAX + 1], cmd[STRMAX * 2 + 20];

	*to = '\0';
	if (getarg("Mail To: ", to, STRMAX))
		return;
	*subject = '\0';
	switch (getarg("Subject: ", subject, STRMAX)) {
	case 0:
		break;
	case 1:
		if (ask("Send with empty subject? ") != YES)
			return;
		break;
	case ABORT:
		return;
	}

	echo("Sending...");
	/* Send the mail.
	 * We do not check the return code from the system calls because some
	 * systems (Motorola...) always returns -1 (EINTR).
	 */
	if (*subject)
		sprintf(cmd, "%s -s \"%s\" %s",
			VARSTR(VMAIL), subject, to);
	else
		sprintf(cmd, "%s %s", VARSTR(VMAIL), to);
	bufftopipe(Curbuff, cmd);
	echo("Mail sent.");
}

/* send the current buffer to the printer */
void Zprint(void)
{
	char cmd[STRMAX + 20];

	echo("Printing...");
	/* note that BuffToPipe updates cmd */
	strcpy(cmd, VARSTR(VPRINT));
	printexit(bufftopipe(Curbuff, cmd));
}

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

/* beautify the current buffer */
#define INDENT		"indent"

void Zbeauty(void)
{
	char cmdStr[128];
	char fileName1[25], fileName2[25];
	int status, fd1;

	Arg = 0;
	if (!(Curbuff->bmode & PROGMODE)) {
		echo("Not a program buffer!");
		return;
	}
	echo("Beautifying...");
	strcpy(fileName1, "/tmp/cbeaut1XXXXXX");
	fd1 = mkstemp(fileName1);
	strcpy(fileName2, fileName1);
	fileName2[11] = '2';

	if (fd1 == -1) {
		echo("mkstemp failed");
		return;
	}
	status = bwritefd(fd1);
	close(fd1);
	if (!status) {
		echo("Write failed");
		return;
	}

#ifdef PIPESH
	sprintf(cmdStr, "%s %s %s", INDENT, fileName1, fileName2);
	if (!dopipe(Curbuff, cmdStr))
		return;

	while (Curbuff->child != EOF)
		checkpipes(2);
#else
	sprintf(cmdStr, "%s %s %s >/dev/null 2>&1", INDENT, fileName1,
		fileName2);
	if (system(cmdStr))
		return;
#endif

	if (access(fileName2, 0)) {
		sprintf(PawStr, "Unable to execute %s.", INDENT);
		error(PawStr);
	} else {
		breadfile(fileName2);
		Curbuff->bmodf = MODIFIED;
		clrecho();
	}

	unlink(fileName1);
	unlink(fileName2);
}

/* Returns -1 if popen failed, else exit code.
 * Leaves Point and Mark where they where.
 */
static int bufftopipe(struct buff *buff, char *cmd)
{
	FILE *pfp;
	struct mark spnt, end;
	struct buff *was = Curbuff;

	strcat(cmd, ">/dev/null 2>&1");
	pfp = popen(cmd, "w");
	if (pfp == NULL)
		return -1;

	bswitchto(buff);
	bmrktopnt(&spnt);	/* save current Point */
	if (Argp) {
		/* Use the region - make sure mark is after Point */
		mrktomrk(&end, Curbuff->mark);
		if (bisaftermrk(&end))
			bswappnt(&end);
	} else {
		/* use entire buffer */
		btoend();
		bmrktopnt(&end);
		btostart();
	}
	Argp = FALSE;
	Arg = 0;

	for (; bisbeforemrk(&end); bmove1())
		putc(Buff(), pfp);

	bpnttomrk(&spnt);
	bswitchto(was);

	return pclose(pfp) >> 8;
}

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
	else {
		sprintf(PawStr, "Exit %d.", code);
		echo(PawStr);
	}
}
#else
void Zshell(void) { tbell(); }
void Zprint(void) { tbell(); }
void Zcmd(void) { tbell(); }
void Zcmdtobuff(void) { tbell(); }
void Zmail(void) { tbell(); }
void Zman(void) { tbell(); }
void Zbeauty(void) { tbell(); }
#endif /* SHELL */
