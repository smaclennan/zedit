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
	First = false; /* used by pinsert when InPaw */
}

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

/* echo 'str' to the paw and as the filename for 'buff' */
static void message(struct buff *buff, const char *str)
{
	struct wdo *wdo;

	if (buff->fname)
		free(buff->fname);
	buff->fname = strdup(str);
	foreachwdo(wdo)
		if (wdo->wbuff == buff)
			wdo->modeflags = INVALID;
	putpaw("%s", str);
}

#if DOPIPES
#include <signal.h>
#include <sys/wait.h>

static int npipes;
static int Waiting;
static fd_set SelectFDs;
static int NumFDs;

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
static bool dopipe(struct buff *tbuff, const char *icmd)
{
	char cmd[STRMAX + 1], *p, *argv[11];
	int from[2], arg;

	if (tbuff->child != EOF) {
		error("%s in use....", tbuff->bname);
		return false;
	}

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

static void cmdtobuff(const char *bname, const char *cmd)
{
	struct wdo *save = Curwdo;

	if (wuseother(bname)) {
		do_chdir(Curbuff);
		if (dopipe(Curbuff, cmd))
			message(Curbuff, cmd);

		wswitchto(save);
	}
}

void Zkill(void)
{
	unvoke(cfindbuff(MAKEBUFF), false);
}
#else
void Zkill(void) { tbell(); }

void execute(void)
{
	zrefresh();
	dotty();
}

static void cmdtobuff(const char *bname, const char *cmdin)
{
	FILE *pfp;
	int rc;
	struct wdo *save = Curwdo;
	char cmd[PATHMAX], line[STRMAX];
	snprintf(cmd, sizeof(cmd), "%s 2>&1", cmdin);

	pfp = popen(cmd, "r");
	if (pfp == NULL) {
		error("Unable to execute %s.", cmd);
		return;
	}

	do_chdir(Curbuff);
	if (wuseother(bname)) {
		message(Curbuff, cmdin);
		putpaw("Please wait...");
		while (fgets(line, sizeof(line), pfp)) {
			binstr(line);
			zrefresh();
		}
	}

	rc = pclose(pfp) >> 8;
	if (rc == 0) {
		btostart();
		putpaw("Done.");
	} else
		putpaw("Returned %d", rc);
	wswitchto(save);
}
#endif /* DOPIPES */

/* This is cleared in Zmake and set in Znexterror.
 * If clear, the make buffer is scrolled up. Once a next error is
 * called, the buffer is kept at the error line.
 */
static int NexterrorCalled;

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

static int set_cmd(int which, const char *prompt)
{
	char cmd[STRMAX + 1];

	Argp = false;
	strcpy(cmd, VARSTR(which));
	if (_getarg(prompt, cmd, STRMAX, false))
		return 0;
	VARSTR(which) = strdup(cmd);
	return 1;
}

static void do_make(const char *cmd)
{
	NexterrorCalled = 0;	/* reset it */
	Arg = 0;

	saveall(true);
	cmdtobuff(MAKEBUFF, cmd);
}

void Zmake(void)
{
	if (Argp)
		if (!set_cmd(VMAKE, "Make: "))
			return;

	do_make(VARSTR(VMAKE));
}

void Zgrep(void)
{
	char cmd[STRMAX * 3], input[STRMAX + 1], files[STRMAX + 1];

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

	do_make(cmd);
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

void Zcmd_to_buffer(void)
{
	static char cmd[STRMAX + 1];

	Arg = 0;
	if (getarg("@ ", cmd, STRMAX) == 0)
		cmdtobuff(SHELLBUFF, cmd);
}
