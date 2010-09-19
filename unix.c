#include "z.h"

#include <signal.h>
#if SYSV4
#include <sys/wait.h>
#endif

/* Come here on SIGHUP or SIGTERM */
void Hangup(int signal)
{
	Buffer *bsave, *tbuff;

	InPaw = TRUE;	/* Kludge to turn off Error */
	bsave = Curbuff;
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next) {
		if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF)) {
			Bswitchto(tbuff);
			Bwritefile(strcmp(Bfname(),
					  MAINBUFF) ? Bfname() : "MAIN.HUP");
		}
#if PIPESH
		if (tbuff->child != EOF)
			Unvoke(tbuff, FALSE);
#endif
	}
#if PIPESH
	Checkpipes(0);
#endif
	Save(bsave);
	Tfini();
	exit(1);
}

#if PIPESH
static int Readapipe(Buffer *);

int Waiting;
fd_set SelectFDs;
int NumFDs;


/* Read all the active pipe buffers. */
int Readpipes(fd_set *fds)
{
	Buffer *tbuff;
	int did_something = 0;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (tbuff->child != EOF && FD_ISSET(tbuff->in_pipe, fds)) {
			Readapipe(tbuff);
			++did_something;
		}

	return did_something;
}

/* Wait for dead children and cleanup.
 *		type == 0 on exit
 *		type == 1 on normal
 *		type == 2 on blocking
 */
int Checkpipes(int type)
{
	Buffer *tbuff;
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
					while (Readapipe(tbuff) > 0)
						;
				FD_CLR(tbuff->in_pipe, &SelectFDs);
				/* SAM Should reduce NumFDs */
				(void)close(tbuff->in_pipe);
				tbuff->in_pipe = EOF;
				if (tbuff->out_pipe)
					fclose(tbuff->out_pipe);
				tbuff->out_pipe = NULL;
				tbuff->child = EOF;
				if (type == 1) {
					if (status & 0xff)
						Message(tbuff, "Died.");
					else {
						status = status >> 8 & 0xff;
						if (status == 0)
							Message(tbuff, "Done.");
						else {
							sprintf(PawStr,
								"Exit %d.",
								status);
							Message(tbuff, PawStr);
						}
					}
					Tbell();
				}
				break;
			}
	}

#if SYSV2
	/* See note in Sigchild() */
#if !SYSV4 || !defined(WNOWAIT)
	signal(SIGCLD, Sigchild);
#endif
	signal(SIGPIPE, Sigchild);
#endif
	return pid;
}

/* pipe has something for us */
static int Readapipe(Buffer *tbuff)
{
	char buff[BUFSIZ], *ptr;
	int cnt, i;

	cnt = i = read(tbuff->in_pipe, ptr = buff, BUFSIZ);
	if (i > 0) {
		/* Yup! Read somethin' */
		struct mark tmark;
		Buffer *save = Curbuff;

		Bswitchto(tbuff);
		Bmrktopnt(&tmark);
		Btoend();
		while (i-- > 0)
			Binsert(*ptr++);
		if (tbuff->out_pipe)
			Bmrktopnt(Curbuff->mark);
		else
			Bpnttomrk(&tmark);
		Bswitchto(save);
	} else
		/* pipe died */
		Checkpipes(1);
	return cnt;
}

/*
Send the buffer line to a pipe.
This command is invoked by the Newline command.
*/
void Sendtopipe()
{
	char line[256 + 1];
	int i;
	struct mark tmark;

	Mrktomrk(&tmark, Curbuff->mark);
	if (Bisaftermrk(&tmark))
		Bswappnt(&tmark);
	for (i = 0; i < 256 && !Bisatmrk(&tmark); Bmove1(), ++i)
		line[i] = Buff();
	line[i] = '\0';
	fputs(line, Curbuff->out_pipe);
	fflush(Curbuff->out_pipe);
	if (!Bisend()) {
		Btoend();
		Binstr(line);
	}
}

/*
Invoke a shell on the other end of a two way pipe.
Returns true if the invocation succeeded.
*/
Boolean Invoke(Buffer *tbuff, char *argv[])
{
	int from[2], to[2];

	/* Zshell may call with tbuff->child not EOF */
	if (tbuff->child != EOF)
		return FALSE;

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
				write(from[1], "Unable to exec shell", 20);
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
				return TRUE;
			} else {
				/* fork failed - clean up */
				(void)close(from[0]);
				(void)close(to[1]);
				Error("Unable to fork shell");
			}
		} else {
			(void)close(from[0]);
			(void)close(from[1]);
		}
	}
	Error("Unable to open pipes");
	return FALSE;
}

/* Invoke 'cmd' on a pipe.
 * Returns true if the invocation succeeded.
*/
Boolean Dopipe(Buffer *tbuff, char *icmd)
{
	char cmd[STRMAX + 1], *p, *argv[11];
	int from[2], arg;

	if (tbuff->child != EOF)
		return FALSE;

	strcpy(p = cmd, icmd);
	for (arg = 0; arg < 10 && (argv[arg] = Wordit(&p)); ++arg)
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
			return TRUE;
		} else {
			/* fork failed - clean up */
			(void)close(from[0]);
			Error("Unable to fork shell");
		}
	} else
		Error("Unable to open pipes");
	return FALSE;
}

/* Split a string up into words.
 * A single quoted string (e.g. 'a b c') is
 * considered one word.
 */
char *Wordit(char **str)
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

/* Try to kill a child process */
void Unvoke(Buffer *child, Boolean check)
{
	if (child && child->child != EOF) {
		kill(child->child, SIGKILL);
		if (check)
			while (child->child != EOF && Checkpipes(1) != -1)
				;
	} else
		Tbell();
}

/* Come here when a child dies or exits.
 *
 * NOTE:For system 3 and system 5: After coming here we do not rebind the
 *		signals to Sigchild. We wait until the Checkpipes routine. If we
 *		do it here, the system seems to send us infinite SIGCLDs.
 */
void Sigchild(int signo)
{
	++Waiting;
}
#endif

#if BSD
#include <sys/stat.h>

int access(char *path, int mode)
{
	int rc;
	struct stat s;

	if (stat(path, &s) == 0) {
		errno = EPERM;
		switch (mode) {
		case 0:
			return 0;
		case 2:
			if (s.st_uid == Me->pw_uid)
				rc = s.st_mode & 0200;
			else if (s.st_gid == Me->pw_gid)
				rc = s.st_mode & 020;
			else
				rc = s.st_mode & 02;
			return rc ? 0 : EOF;
		}
	}
	return EOF;
}
#endif
