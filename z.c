/* z.c - Zedit mainline
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
#include <stdarg.h>

bool Initializing = true;
char *Home;
char *Cwd;
unsigned Cmd;
jmp_buf	zenv;

static char *ConfigDir;
static char dbgfname[PATHMAX];

#ifndef CONFIGDIR
#define CONFIGDIR "/usr/share/zedit"
#endif


static void usage(char *prog)
{
	printf(
		"usage: %s [-ht] [-c config_dir] [-l line] [fname ...]\n"
		"where:\t-h  displays this message.\n"
		"\t-t  default to text mode.\n"
		"\t-c  specifies a config dir.\n"
		"\t-l  goto specified line number. (First file only)\n"
		, prog);

	exit(1);
}

int main(int argc, char **argv)
{
	char path[PATHMAX + 1];
	int arg, files = 0, textMode = 0, exitflag = 0;
	struct buff *tbuff = NULL;

	/* A longjmp is called if bcremrk runs out of memory */
	if (setjmp(zenv) != 0) {
		error("FATAL ERROR: Out of memory");
		Argp = false;	/* so Zexit will not default to save */
		Zexit();
	}

	Home = getenv("HOME");
	if (!Home) {
		struct passwd *pw = getpwuid(getuid());
		if (pw)
			Home = strdup(pw->pw_dir);
		if (!Home) {
			puts("You don't exist!");
			exit(1);
		}
	}

	snprintf(dbgfname, sizeof(dbgfname), "%s/%s", Home, ZDBGFILE);
	unlink(dbgfname);

	Cwd = getcwd(NULL, PATHMAX);
	if (!Cwd) {
		puts("Unable to get CWD");
		exit(1);
	}

	while ((arg = getopt(argc, argv, "c:hl:tE")) != EOF)
		switch (arg) {
		case 'c':
			ConfigDir = optarg;
			break;
		case 'l':
			Arg = atoi(optarg);
			Argp = true;
			break;
		case 't':
			textMode = 1;
			break;
		case 'E':
			exitflag = true;
			break;
		case 'h':
		default:
			usage(argv[0]);
		}

	/* Deal with ConfigDir */
	if (!ConfigDir) {
		ConfigDir = getenv("ZPATH");
		if (!ConfigDir)
			ConfigDir = CONFIGDIR;
	}

	readvfile();		/* Do this BEFORE tinit */

	/* User wants Text mode as default */
	if (textMode)
		VAR(VNORMAL) = 0;

	initscrnmarks(); /* init the screen marks and mark list */

	/* create the needed buffers */
	Killbuff = bcreate();
	Paw = bcreate();
	if (!cmakebuff(MAINBUFF, NULL)) {
		puts("Not enough memory.");
		exit(1);
	}
	Paw->bname = PAWBUFNAME;

	tinit();

	REstart	= bcremrk();
	Sstart	= bcremrk();
	Psstart	= bcremrk();
	Send	= bcremrk();
	Sendp	= false;

	for (; optind < argc; ++optind, ++files)
		if (pathfixup(path, argv[optind]) == 0)
			if (findfile(path) && !tbuff)
				tbuff = Curbuff;

	if (tbuff) {
		bswitchto(tbuff);

		strcpy(Lbufname,
		       Curbuff->prev ? Curbuff->prev->bname : MAINBUFF);

		clrpaw();
	}

	winit();

	reframe();

	if (!Curbuff->mtime && Curbuff->fname)
		putpaw("New File");

	/* Do this after tinit */
	if (findpath(path, ZBFILE))
		bindfile(path, READ_MODE);
	else
		bind();

	Curwdo->modeflags = INVALID;

	if (Argp)
		Zgoto_line();

	Initializing = false;

	if (exitflag)
		Zexit();

	while (1)
		execute();
}

void Dbg(char *fmt, ...)
{
	FILE *fp = fopen(dbgfname, "a");
	if (fp) {
		va_list arg_ptr;

		va_start(arg_ptr, fmt);
		vfprintf(fp, fmt, arg_ptr);
		va_end(arg_ptr);
		fclose(fp);
	}
}

static bool isfile(char *path, char *dir, char *fname, bool must)
{
	if (!dir || !fname)
		return false;
	strcpy(path, dir);
	if (!Psep(*(path + strlen(path) - 1)))
		strcat(path, "/");
	strcat(path, fname);
	return !must || access(path, 0) == 0;
}

/* Find the correct path for the config files.
 * We check HOME and then CONFIGDIR.
 */
int findpath(char *p, char *f)
{
	if (isfile(p, Home, f, true))
		return 2;
	else if (isfile(p, ConfigDir, f, true))
		return 1;
	else
		return 0;
}
