/* shell.c - shell commands and routines
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

/* NOTE: Dotty blocks */
static void dotty(void)
{
	Cmd = tgetcmd();
	Arg = 1;
	Argp = false;
	while (Arg > 0) {
		CMD(Keys[Cmd]);
		--Arg;
	}
	Lfunc = Keys[Cmd];
	First = false;				/* used by pinsert when InPaw */
}


#if SHELL
#include <signal.h>
#include <sys/wait.h>

static int npipes;
static int Waiting;
static fd_set SelectFDs;
static int NumFDs;

static void do_chdir(struct buff *buff)
{
	if (buff->fname) {
		char dir[PATHMAX + 1], *p;
		
		strcpy(dir, buff->fname);
		p = strrchr(dir, '/');
		if (p) {
			*p = '\0';
			chdir(dir);
		}
	}
}

/* pipe has something for us */
static int readapipe(struct buff *tbuff)
{
	char buff[BUFSIZ], *ptr;
	int cnt, i;

	cnt = i = read(tbuff->in_pipe, ptr = buff, BUFSIZ);
	if (i > 0) {
		/* Yup! Read somethin' */
		struct mark tmark;
		struct buff *save = Curbuff;

		bswitchto(tbuff);
		bmrktopnt(&tmark);
		btoend();
		while (i-- > 0)
			binsert(*ptr++);
		bpnttomrk(&tmark);
		bswitchto(save);
	} else
		/* pipe died */
		checkpipes(1);
	return cnt;
}

void execute(void)
{
	if (NumFDs == 0) {
		FD_SET(1, &SelectFDs);
		NumFDs = 2;
	}

	zrefresh();

	if (cpushed || npipes == 0)
		dotty();
	else {
		struct buff *tbuff;
		fd_set fds = SelectFDs;

		/* select returns -1 if a child dies (SIGPIPE) -
		 * sigchild handles it */
		while (select(NumFDs, &fds, NULL, NULL, NULL) == -1) {
			checkpipes(1);
			zrefresh();
			fds = SelectFDs;
		}

		npipes = 0;
		for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
			if (tbuff->child != EOF) {
				++npipes;
				if (FD_ISSET(tbuff->in_pipe, &fds))
					readapipe(tbuff);
			}

		if (npipes == 0)
			NumFDs = 2;

		if (FD_ISSET(1, &fds))
			dotty();
	}
}

static void exit_status(struct buff *tbuff, int status)
{
	if (status & 0xff)
		message(tbuff, "Died.");
	else {
		status = status >> 8 & 0xff;
		if (status == 0)
			message(tbuff, "Done.");
		else {
			sprintf(PawStr,
				"Exit %d.",
				status);
			message(tbuff, PawStr);
		}
	}
	tbell();
}

/* Wait for dead children and cleanup.
 *		type == 0 on exit
 *		type == 1 on normal
 *		type == 2 on blocking
 */
int checkpipes(int type)
{
	struct buff *tbuff;
	int pid = 0, status;

	if (type == 2)
		waitpid((pid_t)-1, &status, WNOWAIT);
	while ((pid = waitpid((pid_t)-1, &status, WNOHANG)) > 0) {
		--Waiting;		/* one less to wait for */
		for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
			if (tbuff->child == pid) {
				/*
				 * make sure pipe empty (except on exit)
				 *	- since child is dead, read
				 *	will not block if nothing to
				 *	read
				 */
				if (type)
					while (readapipe(tbuff) > 0)
						;
				FD_CLR(tbuff->in_pipe, &SelectFDs);
				(void)close(tbuff->in_pipe);
				tbuff->in_pipe = EOF;
				tbuff->child = EOF;
				if (type)
					exit_status(tbuff, status);
				break;
			}
	}

#ifdef SYSV4
	/* See note in sigchild() */
#if !defined(WNOWAIT)
	signal(SIGCLD, sigchild);
#endif
	signal(SIGPIPE, sigchild);
#endif
	return pid;
}

/* Split a string up into words.
 * A single quoted string (e.g. 'a b c') is
 * considered one word.
 */
static char *wordit(char **str)
{
	char *start;

	while (isspace(**str))
		++*str;
	if (**str == '\'') {
		start = ++*str;
		while (**str && **str != '\'')
			++*str;
	} else {
		start = *str;
		while (**str && !isspace(**str))
			++*str;
	}
	if (**str)
		*(*str)++ = '\0';
	return *start ? start : NULL;
}

/* Invoke 'cmd' on a pipe.
 * Returns true if the invocation succeeded.
*/
static bool dopipe(struct buff *tbuff, char *icmd)
{
	char cmd[STRMAX + 1], *p, *argv[11];
	int from[2], arg;

	if (tbuff->child != EOF)
		return false;

	strcpy(p = cmd, icmd);
	for (arg = 0; arg < 10 && (argv[arg] = wordit(&p)); ++arg)
		;

	if (pipe(from) == 0) {
		tbuff->child = fork();
		if (tbuff->child == 0) {
			/* child */
			(void)close(from[0]);
			dup2(from[1], 1);
			dup2(from[1], 2);
			execvp(argv[0], argv);
			exit(1);
		}

		(void)close(from[1]);		/* close fail or not */
		if (tbuff->child != EOF) {
			/* SUCCESS! */
			tbuff->in_pipe = from[0];
			FD_SET(from[0], &SelectFDs);
			if (from[0] >= NumFDs)
				NumFDs = from[0] + 1;
			++npipes;
			return true;
		} else {
			/* fork failed - clean up */
			(void)close(from[0]);
			error("Unable to fork shell");
		}
	} else
		error("Unable to open pipes");
	return false;
}

/* Try to kill a child process */
void unvoke(struct buff *child, bool check)
{
	if (child && child->child != EOF) {
		kill(child->child, SIGKILL);
		if (check)
			while (child->child != EOF && checkpipes(1) != -1)
				;
	} else
		tbell();
}

/* Come here when a child dies or exits.
 *
 * NOTE:For system 3 and system 5: After coming here we do not rebind the
 *		signals to sigchild. We wait until the checkpipes routine. If we
 *		do it here, the system seems to send us infinite SIGCLDs.
 */
void sigchild(int signo)
{
	++Waiting;
}

static struct buff *cmdtobuff(char *bname, char *cmd)
{
	struct buff *tbuff = NULL;
	struct wdo *save;

	save = Curwdo;
	do_chdir(Curbuff);
	if (wuseother(bname)) {
		if (dopipe(Curbuff, cmd))
			tbuff = Curbuff;
		wswitchto(save);
	}
	return tbuff;
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

void Zcmd_to_buffer(void)
{
	static char cmd[STRMAX + 1];
	struct wdo *save;
	int rc;

	Arg = 0;
	if (getarg("@ ", cmd, STRMAX) == 0) {
		save = Curwdo;
		do_chdir(Curbuff);
		if (wuseother(SHELLBUFF)) {
			putpaw("Please wait...");
			rc = pipetobuff(Curbuff, cmd);
			if (rc == 0) {
				message(Curbuff, cmd);
				btostart();
				putpaw("Done.");
			} else if (rc == -1)
				putpaw("Unable to execute.");
			else
				putpaw("Exit %d.", rc);
			Curbuff->bmodf = false;
			wswitchto(save);
		}
	}
}

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
	VARSTR(which) = strdup(cmd);
	return 1;
}

void Zmake(void)
{
	struct buff *mbuff;

	NexterrorCalled = 0;	/* reset it */
	Arg = 0;
	if (Argp)
		if (!set_cmd(VMAKE, "Make: "))
			return;
	saveall(true);
	mbuff = cfindbuff(MAKEBUFF);
	if (mbuff && mbuff->child != EOF) {
		putpaw("Killing current make.");
		unvoke(mbuff, true);
		clrpaw();
	}
	mbuff = cmdtobuff(MAKEBUFF, VARSTR(VMAKE));
	if (mbuff)
		message(mbuff, VARSTR(VMAKE));
	else
		error("Unable to execute make.");
}

void Zgrep(void)
{
	struct buff *mbuff;
	char cmd[STRMAX * 3], input[STRMAX + 1], files[STRMAX + 1];

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
	mbuff = cfindbuff(MAKEBUFF);
	if (mbuff && mbuff->child != EOF) {
		error("Make buffer in use...");
		return;
	}
	mbuff = cmdtobuff(MAKEBUFF, cmd);
	if (mbuff)
		message(mbuff, cmd);
	else
		error("Unable to execute grep.");
}

void Znext_error(void)
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
	} else
		bcsearch(NL);
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
		Zgoto_line();
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

void Zkill(void)
{
	unvoke(cfindbuff(MAKEBUFF), false);
}

/* Find the next error in the .make buffer.
 * Ignores lines that start with a white space.
 * Supported: <fname>:<line>:
 */
static int parse(char *fname)
{
	int line, n;

	while (!bisend()) {
		/* try to get the fname */
		n = getbword(fname, PATHMAX, bistoken);
		bmove(n);

		/* try to get the line */
		if (Buff() == ':') {
			bmove1();

			/* look for line number */
			line = batoi();
			if (line)
				return line;
		}

		/* skip to next line */
		bcsearch(NL);
	}
	return 0;
}
#else
void execute(void)
{
	zrefresh();
	dotty();
}

void Zcmd_to_buffer(void) { tbell(); }
void Zmake(void) { tbell(); }
void Znext_error(void) { tbell(); }
void Zkill(void) { tbell(); }
void Zgrep(void) { tbell(); }
#endif /* SHELL */
