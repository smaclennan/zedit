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
#if BSD
#include <signal.h>
#endif

/* Do one shell command to the screen */
#if XWINDOWS
Proc Zcmd() { Tbell(); }	/* no screen */
#elif !BSD
Proc Zcmd()
{
	char tb[STRMAX * 2];

	Arg = 0;
	if (Getarg("! ", Command, STRMAX) == 0) {
		Tfini();
		sprintf(tb, "%s -c \"%s\"", Shell, Command);
		if (system(tb) == EOF)
			Echo("command failed");
		else {
			fputs("\n[Hit Return to continue]", stdout);
			Tgetcmd();
			putchar('\n');
		}
		Tinit();
	}
}
#endif

/* Do one shell command to the .shell buffer */
Proc Zcmdtobuff()
{
#ifdef BORDER3D
	Tbell();
#elif XWINDOWS || PIPESH
	WDO *save;
	int rc;

	Arg = 0;
	if (Getarg("@ ", Command, STRMAX) == 0) {
		save = Curwdo;
		if (WuseOther(SHELLBUFF)) {
			Echo("Please wait...");
			rc = PipeToBuff(Curbuff, Command);
			if (rc == 0) {
				Message(Curbuff, Command);
				Btostart();
			}
			Curbuff->bmodf = FALSE;
			PrintExit(rc);
			Wswitchto(save);
		}
	}
#else
	Arg = 0;
	if (Getarg("@ ", Command, STRMAX) == 0)
		Cmdtobuff(SHELLBUFF, Command);
#endif
}

/* Perform man command on pipe, wait for completion, and format */
Proc Zman()
{
	char entry[STRMAX + 5], *p;
	int rc;
#ifndef BORDER3D
	WDO *save;
#endif

	strcpy(entry, "man ");
	p = entry + strlen(entry);
	Getbword(p, STRMAX, Istoken);	/* get word */
	if (Getarg("Man: ", p, STRMAX))
		return;

#ifndef BORDER3D
	/*  BORDER3D always pops up man page */
	if (Vars[VPOPMAN].val) {
#endif
		Buffer *buff;
		FILE *pfp;

		sprintf(PawStr, "show -t \"man %s\"", p);
		buff = Bcreate();
		pfp = popen(PawStr, "w");
		if (buff && pfp) {
			Buffer *bsave = Curbuff;
			Bswitchto(buff);
			Echo("Please wait...");
			rc = PipeToBuff(buff, entry);
			if (rc == 0) {
				/* remove the underlines */
				Btoend();
				while (Bcrsearch('\010')) {
					Bmove(-1);
					Bdelete(2);
				}

				for (Btostart(); !Bisend(); Bmove1())
					fputc(Buff(), pfp);
			}
			Bswitchto(bsave);
			pclose(pfp);
			Bdelbuff(buff);
			PrintExit(rc);
			return;
		} else {
			if (buff)
				Bdelbuff(buff);
			Echo("\7Unable to popup man page.");
		}
	}

#ifndef BORDER3D
	save = Curwdo;
	if (WuseOther(MANBUFF)) {
		Echo("Please wait...");
		rc = PipeToBuff(Curbuff, entry);
		if (rc == 0) {
			/* remove the underlines */
			Message(Curbuff, p);
			Btoend();
			while (Bcrsearch('\010')) {
				Bmove(-1);
				Bdelete(2);
			}
		}
		Curbuff->bmodf = FALSE;
		Curbuff->bmode |= VIEW;
		PrintExit(rc);
		Wswitchto(save);
	}
#endif
}

#if BSD
#if PIPESH
Proc Zcmd()
#else
Proc Zshell()	/*for tags*/
#endif
{
	Tfini();
	kill(getpid(), SIGTSTP);
}
#endif

#ifdef BORDER3D
Proc Zshell() { Tbell(); }
#elif PIPESH
Proc Zshell()
{
	char bname[BUFNAMMAX + 1];
	int i = 0;

	/* create a unique buffer name */
	if (Cfindbuff(strcpy(bname, SHELLBUFF)))
		do
			sprintf(bname, "%s%d", SHELLBUFF, ++i);
		while (Cfindbuff(bname));

	if (!WuseOther(bname) || !Doshell())
		Tbell();
}

Boolean Doshell()
{
	char *argv[3];

	argv[0] = Shell;
	argv[1] = "-i";
	argv[2] = NULL;
	return Invoke(Curbuff, argv);
}
#elif SYSV2
Proc Zshell()	/*for tags*/
{
	int err = EOF;

	Arg = 0;
	Tfini();
	if (system(Shell) == EOF)
		err = errno;
	Tinit();
	if (err != EOF)
		Syerr(err);
}
#endif

/* send the current buffer as a mail buffer */
Proc Zmail()
{
	char to[STRMAX + 1], subject[STRMAX + 1], cmd[STRMAX * 2 + 20];

	*to = '\0';
	if (Getarg("Mail To: ", to, STRMAX))
		return;
	*subject = '\0';
	switch (Getarg("Subject: ", subject, STRMAX)) {
	case 0:
		break;
	case 1:
		if (Ask("Send with empty subject? ") != YES)
			return;
		break;
	case ABORT:
		return;
	}

	Echo("Sending...");
	/* Send the mail.
	 * We do not check the return code from the system calls because some
	 * systems (Motorola...) always returns -1 (EINTR).
	 */
	if (*subject)
		sprintf(cmd, "%s -s \"%s\" %s",
			(char *)Vars[VMAIL].val, subject, to);
	else
		sprintf(cmd, "%s %s", (char *)Vars[VMAIL].val, to);
	BuffToPipe(Curbuff, cmd);
	Echo("Mail sent.");
}

/* send the current buffer to the printer */
Proc Zprint()
{
	char cmd[STRMAX + 20];

	Echo("Printing...");
	/* note that BuffToPipe updates cmd */
	strcpy(cmd, (char *)Vars[VPRINT].val);
	PrintExit(BuffToPipe(Curbuff, cmd));
}

#ifdef BORDER3D
Buffer *Cmdtobuff(char *bname, char *cmd)
{
	return 0;
}
#elif PIPESH || XWINDOWS
Buffer *Cmdtobuff(char *bname, char *cmd)
{
	Buffer *tbuff = NULL;
	WDO *save;

	save = Curwdo;
	if (WuseOther(bname)) {
		if (Dopipe(Curbuff, cmd))
			tbuff = Curbuff;
		Wswitchto(save);
	}
	return tbuff;
}
#else
Buffer *Cmdtobuff(char *bname, char *cmd)
{
	char fname[20];
	int err, one;
	Buffer *sbuff, *tbuff;

	Arg = Argp = 0;
	Echo("Working...");
	mktemp(strcpy(fname, ZSHFILE));
	err = Dopipe(fname, cmd);
	if (err)
		Syerr(err);
	else {
		WuseOther(bname);
		Breadfile(fname);
		unlink(fname);
		Curbuff->bmodf = FALSE;
		Clrecho();
	}
	return err ? NULL : tbuff;
}
#endif

#if !XWINDOWS && !PIPESH
int Dopipe(char *fname, char *cmd)
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

Proc Zbeauty()
{
	char cmdStr[128];
	char fileName1[25], fileName2[25];
	int status, fd1;

	Arg = 0;
	if (!(Curbuff->bmode & PROGMODE)) {
		Echo("Not a program buffer!");
		return;
	}
	Echo("Beautifying...");
	strcpy(fileName1, "/tmp/cbeaut1XXXXXX");
	fd1 = mkstemp(fileName1);
	strcpy(fileName2, fileName1);
	fileName2[11] = '2';

	if (fd1 == -1) {
		Echo("mkstemp failed");
		return;
	}
	status = Bwritefd(fd1);
	close(fd1);
	if (!status) {
		Echo("Write failed");
		return;
	}

#if PIPESH
	sprintf(cmdStr, "%s %s %s", INDENT, fileName1, fileName2);
	if (!Dopipe(Curbuff, cmdStr))
		return;

	while (Curbuff->child != EOF)
		Checkpipes(2);
#else
	sprintf(cmdStr, "%s %s %s >/dev/null 2>&1", INDENT, fileName1,
		fileName2);
	if (system(cmdStr))
		return;
#endif

	if (access(fileName2, 0)) {
		sprintf(PawStr, "Unable to execute %s.", INDENT);
		Error(PawStr);
	} else {
		Breadfile(fileName2);
		Curbuff->bmodf = MODIFIED;
		Clrecho();
	}

	unlink(fileName1);
	unlink(fileName2);
}

void Syerr(err)
int err;
{
	switch (err) {
	case E2BIG:
	case ENOMEM:
		Error("Not enough memory");
		break;

	case ENOENT:
		Error("Command not found");
		break;

	default:
		Error("Unable to execute");
	}
}

/* Echo 'str' to the paw and as the filename for 'buff' */
void Message(Buffer *buff, char *str)
{
#ifndef BORDER3D
	WDO *wdo;
#endif

	if (buff->fname)
		free(buff->fname);
	buff->fname = strdup(str);
#ifdef BORDER3D
	Curwdo->modeflags = INVALID;
#else
	for (wdo = Whead; wdo; wdo = wdo->next)
		if (wdo->wbuff == buff)
			wdo->modeflags = INVALID;
#endif
	Echo(str);
}

/* Returns -1 if popen failed, else exit code.
 * Leaves Point and Mark where they where.
 */
int BuffToPipe(Buffer *buff, char *cmd)
{
	FILE *pfp;
	Mark spnt, end;
	Buffer *was = Curbuff;

	strcat(cmd, ">/dev/null 2>&1");
	pfp = popen(cmd, "w");
	if (pfp == NULL)
		return -1;

	Bswitchto(buff);
	Bmrktopnt(&spnt);	/* save current Point */
	if (Argp) {
		/* Use the region - make sure mark is after Point */
		Mrktomrk(&end, Curbuff->mark);
		if (Bisaftermrk(&end))
			Bswappnt(&end);
	} else {
		/* use entire buffer */
		Btoend();
		Bmrktopnt(&end);
		Btostart();
	}
	Argp = FALSE;
	Arg = 0;

	for (; Bisbeforemrk(&end); Bmove1())
		putc(Buff(), pfp);

	Bpnttomrk(&spnt);
	Bswitchto(was);

	return pclose(pfp) >> 8;
}

/* Returns -1 if popen failed, else exit code.
 * Leaves Point at end of new text.
 */
int PipeToBuff(Buffer *buff, char *instr)
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
		Binsert((char)c);
	free(cmd);
	return pclose(pfp) >> 8;
}

void PrintExit(int code)
{
	if (code == 0)
		Echo("Done.");
	else if (code == -1)
		Echo("Unable to execute.");
	else {
		sprintf(PawStr, "Exit %d.", code);
		Echo(PawStr);
	}
}
