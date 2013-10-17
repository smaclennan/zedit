/* unix.c - Zedit Unix specific routines
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

#include <signal.h>
#include <sys/wait.h>

/* Come here on SIGHUP or SIGTERM */
void hang_up(int signal)
{
	struct buff *tbuff;

	InPaw = true;	/* Kludge to turn off error */
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next) {
		if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF)) {
			bswitchto(tbuff);
			bwritefile(strcmp(bfname(),
					  MAINBUFF) ? bfname() : "MAIN.HUP");
		}
#ifdef PIPESH
		if (tbuff->child != EOF)
			unvoke(tbuff, false);
#endif
	}
#ifdef PIPESH
	checkpipes(0);
#endif
	tfini();
	exit(1);
}

#ifdef PIPESH
static int readapipe(struct buff *);

static int Waiting;
fd_set SelectFDs;
int NumFDs;


/* Read all the active pipe buffers. */
int readpipes(fd_set *fds)
{
	struct buff *tbuff;
	int did_something = 0;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (tbuff->child != EOF && FD_ISSET(tbuff->in_pipe, fds)) {
			readapipe(tbuff);
			++did_something;
		}

	return did_something;
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
				/* SAM Should reduce NumFDs */
				(void)close(tbuff->in_pipe);
				tbuff->in_pipe = EOF;
				if (tbuff->out_pipe)
					fclose(tbuff->out_pipe);
				tbuff->out_pipe = NULL;
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
		if (tbuff->out_pipe)
			bmrktopnt(Curbuff->mark);
		else
			bpnttomrk(&tmark);
		bswitchto(save);
	} else
		/* pipe died */
		checkpipes(1);
	return cnt;
}

/*
Send the buffer line to a pipe.
This command is invoked by the Newline command.
*/
void sendtopipe(void)
{
	char line[256 + 1];
	int i;
	struct mark tmark;

	mrktomrk(&tmark, Curbuff->mark);
	if (bisaftermrk(&tmark))
		bswappnt(&tmark);
	for (i = 0; i < 256 && !bisatmrk(&tmark); bmove1(), ++i)
		line[i] = Buff();
	line[i] = '\0';
	fputs(line, Curbuff->out_pipe);
	fflush(Curbuff->out_pipe);
	if (!bisend()) {
		btoend();
		binstr(line);
	}
}

/*
Invoke a shell on the other end of a two way pipe.
Returns true if the invocation succeeded.
*/
bool invoke(struct buff *tbuff, char *argv[])
{
	int from[2], to[2];

	/* Zshell may call with tbuff->child not EOF */
	if (tbuff->child != EOF)
		return false;

	if (pipe(from) == 0) {
		if (pipe(to) == 0) {
			tbuff->child = fork();
			if (tbuff->child == 0) {
				/* child */
				(void)close(from[0]);
				(void)close(to[1]);
				dup2(to[0],   0);
				dup2(from[1], 1);
				dup2(from[1], 2);
				execvp(argv[0], argv);
				fputs("Unable to exec shell\n", stderr);
				pause();	/* wait to die */
			}

			(void)close(from[1]);	/* we close these fail or not */
			(void)close(to[0]);
			if (tbuff->child != EOF) {
				/* SUCCESS! */
				tbuff->in_pipe  = from[0];
				tbuff->out_pipe = fdopen(to[1], "w");
				FD_SET(from[0], &SelectFDs);
				if (from[0] >= NumFDs)
					NumFDs = from[0] + 1;
				return true;
			} else {
				/* fork failed - clean up */
				(void)close(from[0]);
				(void)close(to[1]);
				error("Unable to fork shell");
			}
		} else {
			(void)close(from[0]);
			(void)close(from[1]);
		}
	}
	error("Unable to open pipes");
	return false;
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
bool dopipe(struct buff *tbuff, char *icmd)
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
#endif
