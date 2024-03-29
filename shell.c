/* Copyright (C) 1988-2018 Sean MacLennan */

#include "z.h"

/** @addtogroup zedit
 * @{
 */

/* This is cleared indirectly in Zgrep/cmdtobuff via set_shell_mark()
 * and set in Znexterror.  If clear, the make buffer is scrolled
 * up. Once a next error is called, the buffer is kept at the error
 * line.
 */
static int NexterrorCalled;
static struct mark *shell_mark;

void set_shell_mark(void)
{
	NexterrorCalled = 0;
	if (!shell_mark)
		/* not the end of the world if this fails */
		shell_mark = bcremark(Bbuff);
	else
		bmrktopnt(Bbuff, shell_mark);
}

/* Convert the next portion of buffer to integer. Skip leading ws. */
int batoi(void)
{
	int num;

	while (Buff() == ' ' || Buff() == '\t')
		bmove1(Bbuff);
	for (num = 0; isdigit(Buff()); bmove1(Bbuff))
		num = num * 10 + Buff() - '0';
	return num;
}

int do_chdir(struct zbuff *buff)
{
	if (buff->fname) {
		char dir[PATHMAX + 1], *p;

		strlcpy(dir, buff->fname, sizeof(dir));
		p = strrchr(dir, '/');
		if (p) {
			*p = '\0';
			return chdir(dir);
		}
	}
	return 0;
}

/* echo 'str' to the paw and as the filename for 'buff' */
void message(struct zbuff *buff, const char *str)
{
	struct wdo *wdo;

	free(buff->fname);
	buff->fname = strdup(str);
	foreachwdo(wdo)
		if (wdo->wbuff == buff)
			wdo->modeflags = INVALID;
	putpaw("%s", str);
}

#ifdef DOPIPES
#include <signal.h>
#include <sys/wait.h>

/* We only allow one pipe command */
static struct zbuff *pipebuff;
static pid_t child = EOF;	/* PID of shell or EOF */
static int in_pipe = -1;	/* the pipe */

/* Come here when a child dies or exits.
 *
 * NOTE:For system 3 and system 5: After coming here we do not rebind the
 *		signals to sigchild. We wait until the checkpipes routine. If we
 *		do it here, the system seems to send us infinite SIGCLDs.
 */
static void sigchild(int signo) {}

void siginit(void)
{
#ifndef WNOWAIT
#ifdef SIGCHLD
	signal(SIGCHLD,  sigchild);
#else
	signal(SIGCLD,  sigchild);
#endif
#endif
	signal(SIGPIPE, sigchild);
}

/* pipe has something for us */
int readapipe(void)
{
	char buff[1024], *ptr;
	int cnt, i;

	if (in_pipe == -1)
		return 0;
	cnt = i = read(in_pipe, ptr = buff, sizeof(buff));
	if (i > 0) {
		/* Yup! Read somethin' */
		struct mark tmark;
		struct zbuff *save = Curbuff;

		zswitchto(pipebuff);
		bmrktopnt(Bbuff, &tmark);
		btoend(Bbuff);
		while (i-- > 0)
			binsert(Bbuff, *ptr++);
		bpnttomrk(Bbuff, &tmark);
		zswitchto(save);
	} else
		/* pipe died */
		checkpipes(1);
	return cnt;
}

static void exit_status(struct zbuff *tbuff, int status)
{
	if (status & 0xff)
		message(tbuff, "Died.");
	else {
		status = status >> 8 & 0xff;
		if (status == 0)
			message(tbuff, "Done.");
		else {
			strfmt(PawStr, PAWSTRLEN, "Exit %d.", status);
			message(tbuff, PawStr);
		}
	}
	if (status)
		tbell();
}

/* Wait for dead children and cleanup. Type == 0 on exit. */
void checkpipes(int type)
{
	int pid = 0, status;

	if (child == EOF)
		return;

	pid = waitpid((pid_t)-1, &status, WNOHANG);
	if (pid > 0) {
		if (pid == child) {
			/*
			 * make sure pipe empty (except on exit)
			 *	- since child is dead, read
			 *	will not block if nothing to
			 *	read
			 */
			if (type) {
				while (readapipe() > 0)
					;
				exit_status(pipebuff, status);
			}
			set_pipefd(-1);
			(void)close(in_pipe);
			in_pipe = EOF;
			child = EOF;
			pipebuff = NULL;
		}
	}

#ifdef SYSV4
	siginit();
#endif
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
static bool _cmdtobuff(struct zbuff *tbuff, const char *icmd)
{
	char cmd[STRMAX + 1], *p, *argv[11];
	int from[2], arg;

	if (child != EOF) {
		error("%s in use....", tbuff->bname);
		return false;
	}

	strlcpy(cmd, icmd, sizeof(cmd));
	for (p = cmd, arg = 0; arg < 10 && (argv[arg] = wordit(&p)); ++arg)
		;

	if (pipe(from)) {
		error("Unable to open pipes");
		return false;
	}

	pid_t pid = fork();
	if (pid == 0) {
		/* child */
		(void)close(from[0]);
		dup2(from[1], 1);
		dup2(from[1], 2);
		execvp(argv[0], argv);
		exit(1);
	}

	(void)close(from[1]);		/* close fail or not */
	if (pid == EOF) {
		/* fork failed - clean up */
		(void)close(from[0]);
		error("Unable to fork shell");
		return false;
	}

	/* SUCCESS! */
	pipebuff = tbuff;
	in_pipe = from[0];
	child = pid;
	set_pipefd(in_pipe);
	return true;
}

/* Try to kill a child process */
bool unvoke(struct zbuff *buff)
{
	if (!pipebuff || (buff && buff != pipebuff))
		return false;

	kill(child, SIGKILL);
	return true;
}

void Zkill(void)
{
	if (!unvoke(NULL))
		tbell();
}
#else
#undef EOF
#include <stdio.h>

void Zkill(void) { tbell(); }
bool unvoke(struct zbuff *child) { ((void)child); return false; }
void checkpipes(int type) { ((void)type); }

static void _cmdtobuff(struct zbuff *buff, const char *cmdin)
{
	FILE *pfp;
	int rc;
	char cmd[PATHMAX], line[STRMAX];
	strconcat(cmd, sizeof(cmd), cmdin, "2>&1", NULL);

	pfp = popen(cmd, "r");
	if (pfp == NULL) {
		error("Unable to execute %s.", cmd);
		return;
	}

	putpaw("Please wait...");
	while (fgets(line, sizeof(line), pfp)) {
		binstr(buff->buff, line);
		zrefresh();
	}

	rc = pclose(pfp) >> 8;
	if (rc == 0) {
		btostart(buff->buff);
		putpaw("Done.");
	} else
		putpaw("Returned %d", rc);
}
#endif /* DOPIPES */

static void cmdtobuff(const char *bname, const char *cmd)
{
	struct wdo *save = Curwdo;

	if (wuseother(bname)) {
		set_shell_mark();

		do_chdir(Curbuff);
		message(Curbuff, cmd);

		_cmdtobuff(Curbuff, cmd);

		wswitchto(save);
	}
}

void Zmake(void)
{
	static char mkcmd[STRMAX + 1];

	if (!*mkcmd) {
		strlcpy(mkcmd, VARSTR(VMAKE), sizeof(mkcmd));
		VARSTR(VMAKE) = mkcmd;
	}

	if (Argp) {
		Argp = false;
		if (_getarg("Make: ", mkcmd, STRMAX, false))
			return;
	}

	NexterrorCalled = 0;	/* reset it */
	Arg = 0;

	saveall(true);
	cmdtobuff(SHELLBUFF, mkcmd);
}

void Zcmd_to_buffer(void)
{
	static char cmd[STRMAX + 1];

	Arg = 0;
	if (getarg("@ ", cmd, STRMAX) == 0)
		cmdtobuff(SHELLBUFF, cmd);
}


/* Find the next error in the shell buffer.
 * Ignores lines that start with a white space.
 * Supported: <fname>:<line>:
 */
static int parse(char *fname)
{
	int line;
	char *p;

	while (!bisend(Bbuff)) {
		/* try to get the fname */
		p = fname;
		while (!isspace(Buff()) && Buff() != ':') {
			*p++ = Buff();
			bmove1(Bbuff);
		}
		*p = '\0';

		/* try to get the line */
		if (Buff() == ':') {
			bmove1(Bbuff);

			/* look for line number */
			line = batoi();
			if (line)
				return line;
		}

		/* skip to next line */
		bcsearch(Bbuff, NL);
	}
	return 0;
}

void Znext_error(void)
{
	struct wdo *wdo;
	struct zbuff *save, *mbuff;
	char fname[STRMAX + 1];
	char path[PATHMAX + 1];
	int line;

	mbuff = cfindbuff(SHELLBUFF);
	if (!mbuff) {
		tbell();
		return;
	}
	save = Curbuff;
	zswitchto(mbuff);
	if (!NexterrorCalled) {
		NexterrorCalled = 1;
		btostart(Bbuff);
	} else
		bcsearch(Bbuff, NL);
	line = parse(fname);
	if (line) {
		vsetmrk(shell_mark);
		bmrktopnt(Bbuff, shell_mark);
		tobegline(Bbuff);
		bswappnt(Bbuff, shell_mark);
		vsetmrk(shell_mark);
		wdo = findwdo(mbuff->buff);
		if (wdo)
			mrktomrk(wdo->wstart, shell_mark);
		pathfixup(path, fname);
		findfile(path);
		Argp = true;
		Arg = line;
		Zgoto_line();
		tobegline(Bbuff);
	} else {
		btoend(Bbuff);
		bdelmark(shell_mark);
		shell_mark = NULL;
		zswitchto(save);
		putpaw("No more errors");
	}
	Argp = false;
	Arg = 0;
}

#if SPELL
#include <aspell.h>
#include <dlfcn.h>

#define ZDLSYM(A, B, args...) (A(*B)(args) = (A(*)(args))dlsym(dl, #B))

void Zspell_word(void)
{
	void *dl = dlopen("libaspell.so", RTLD_LAZY);
	if (!dl) {
		error("You do not have libaspell.so");
		return;
	}

	dlerror(); /* clear errors */
	ZDLSYM(AspellConfig *, new_aspell_config);
	ZDLSYM(void, aspell_config_replace,
		   AspellConfig*, char*, char*);
	ZDLSYM(AspellCanHaveError *, new_aspell_speller,
		   AspellConfig*);
	ZDLSYM(int, aspell_error_number,
		   AspellCanHaveError*);
	ZDLSYM(AspellSpeller *, to_aspell_speller,
		   AspellCanHaveError*);
	ZDLSYM(int, aspell_speller_check,
		   AspellSpeller*, char*, int);
	ZDLSYM(const AspellWordList *, aspell_speller_suggest,
		   AspellSpeller*, char*, int);
	ZDLSYM(AspellStringEnumeration *, aspell_word_list_elements,
		   const AspellWordList*);
	ZDLSYM(const char *, aspell_string_enumeration_next,
		   AspellStringEnumeration*);
	ZDLSYM(void, delete_aspell_string_enumeration,
		   AspellStringEnumeration*);
	ZDLSYM(void, delete_aspell_speller,
		   AspellSpeller*);
	ZDLSYM(void, delete_aspell_config,
		   AspellConfig*);
	if (dlerror()) {
		error("You have an incomplete libaspell.so");
		dlclose(dl);
		return;
	}

	AspellConfig * config = new_aspell_config();
	if (!config) {
		error("Aspell failed");
		dlclose(dl);
		return;
	}

	aspell_config_replace(config, (char *)"lang", (char *)"en_US");

	AspellCanHaveError *possible_err = new_aspell_speller(config);
	if (aspell_error_number(possible_err)) {
		error("Aspell failed");
		dlclose(dl);
		return;
	}
	AspellSpeller * speller = to_aspell_speller(possible_err);

	char word[STRMAX];
	getbword(word, sizeof(word), bisword);
	int size = strlen(word);
	int correct = aspell_speller_check(speller, word, size);

	if (correct)
		putpaw("%s correct", word);
	else {
		int n, i = 0, len = Colmax;

		const AspellWordList *suggestions =
			aspell_speller_suggest(speller, word, size);
		AspellStringEnumeration *elements =
			aspell_word_list_elements(suggestions);
		const char *s;
		while ((s = aspell_string_enumeration_next(elements)) &&
			   len > 2) {
			if (i) {
				PawStr[i++] = ' ';
				len--;
			}
			n = strlcpy(PawStr + i, s, len);
			if (n > len)
				n = len; /* overflow */
			i += n;
			len -= n;
		}
		delete_aspell_string_enumeration(elements);

		putpaw(PawStr);
	}

	delete_aspell_speller(speller);
	delete_aspell_config(config);
}
#else
void Zspell_word(void) { tbell(); }
#endif
/* @} */
