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

#if SHELL
#include <signal.h>

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

void Zcmd_to_screen(void)
{
	char tb[STRMAX * 2];

	Arg = 0;
	if (getarg("! ", Command, STRMAX) == 0) {
		tfini();
		sprintf(tb, "%s -c \"%s\"", get_shell(), Command);
		if (system(tb) == EOF)
			putpaw("command failed");
		else {
			fputs("\n[Hit Return to continue]", stdout);
			tgetcmd();
			putchar('\n');
		}
		tinit();
	}
}

void Zcmd_to_buffer(void)
{
	struct wdo *save;
	int rc;

	Arg = 0;
	if (getarg("@ ", Command, STRMAX) == 0) {
		save = Curwdo;
		if (wuseother(SHELLBUFF)) {
			putpaw("Please wait...");
			rc = pipetobuff(Curbuff, Command);
			if (rc == 0) {
				message(Curbuff, Command);
				btostart();
			}
			Curbuff->bmodf = false;
			printexit(rc);
			wswitchto(save);
		}
	}
}

static bool doshell(void)
{
	char *argv[3];

	argv[0] = get_shell();
	argv[1] = "-i";
	argv[2] = NULL;
	return invoke(Curbuff, argv);
}

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
		putpaw("Done.");
	else if (code == -1)
		putpaw("Unable to execute.");
	else
		putpaw("Exit %d.", code);
}
#else
void Zshell(void) { tbell(); }
void Zcmd_to_screen(void) { tbell(); }
void Zcmd_to_buffer(void) { tbell(); }
#endif /* SHELL */
