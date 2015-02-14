/* fcheck.c - Zedit helper program
 * Copyright (C) 2013 Sean MacLennan
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

#define FCHECK
#include "z.h"

#include "varray.c"
#include "funcs.c"
#include "cnames.c"
#include "bind.c"
#ifdef WIN32
#define zrefresh()
HANDLE hstdin;
int Colmax, Rowmax;
#include "winkbd.c"
#else
#undef GPM_MOUSE
#include "kbd.c"
#endif

#if defined __unix__
#define OS unix
#define GITFILE ".git/refs/heads/master"

#include <sys/utsname.h>
static struct utsname utsname;
#elif defined WIN32
#define OS win32
#define GITFILE "../.git/refs/heads/master"
#else
#error Unknown OS
#endif

#define VARSNUM		((int)(sizeof(Vars) / sizeof(struct avar)))

#ifdef __unix__
#define N_KEYS ((int)(sizeof(Tkeys) / sizeof(char *)))
#else
#define N_KEYS NUM_SPECIAL
#endif

int InPaw;
unsigned Cmd;
int ring_bell;
struct wdo *Curwdo;

void hang_up(int sig) { ((void)sig); }

void Dbg(const char *fmt, ...) { ((void)fmt); }

void mouse_scroll(int row, bool down) { ((void)row); ((void)down); }
void mouse_point(int row, int col, bool set_mark) {}

#if ZLIB || SPELL
static int noinclude(int argc, char *argv[], const char *inc)
{
	char path[PATHMAX];
	int arg;

	snprintf(path, sizeof(path), "/usr/include/%s", inc);
	if (access(path, F_OK) == 0)
		return 0; /* found it */

	for (arg = 1; arg < argc; ++arg)
		if (strncmp(argv[arg], "-I", 2) == 0) {
			snprintf(path, sizeof(path), "%s/%s", argv[arg] + 2, inc);
			if (access(path, F_OK) == 0)
				return 0; /* found it */
		}

	return 1; /* not found */
}
#endif

static int compare(char *new, char *old)
{
	int rc = 0;
	FILE *old_fp, *new_fp;
	char new_line[128], old_line[128];

	old_fp = fopen(old, "r");
	if (!old_fp) {
		if (errno != ENOENT)
			perror(old);
		return 1;
	}
	new_fp = fopen(new, "r");
	if (!new_fp) {
		fclose(old_fp);
		perror(new);
		return 1;
	}

	while (fgets(new_line, sizeof(new_line), new_fp))
		if (!fgets(old_line, sizeof(old_line), old_fp) ||
		    strcmp(new_line, old_line)) {
			rc = 1;
			break;
		}

	fclose(new_fp);
	fclose(old_fp);

	return rc;
}

static int build_zversion_h(void)
{
	int mods = 0;
	FILE *fp;
	char version[42];

	strcpy(version, "N/A");

	if ((fp = fopen(GITFILE, "r"))) {
		if (fgets(version, sizeof(version), fp))
			strtok(version, "\r\n");
		fclose(fp);

#ifdef __unix__
		/* Suse doesn't support -s + others */
		fp = popen("git status --untracked-files=no", "r");
		if (fp) {
			char line[80];

			while (fgets(line, sizeof(line), fp))
				if (strncmp(line, "#\t", 2) == 0) {
					mods = 1;
					break;
				}
			fclose(fp);
		} else
			puts("Warning: git status failed.");
#endif
	} else if (errno == ENOENT)
		puts("Warning: .git does not exist or is incomplete.");
	else
		perror(GITFILE);
	
	fp = fopen("zversion.new", "w");
	if (!fp) {
		perror("zversion.new");
		return 1;
	}

	fprintf(fp, "#define ZEDIT_VERSION\t\"%s\"\n", VERSION);
	fprintf(fp, "#define GIT_COMMIT\t\"%s\"\n", version);
	fprintf(fp, "#define GIT_MOD\t\t%d\n", mods);
	fclose(fp);

	if (compare("zversion.new", "zversion.h")) {
		puts("     UPDATE   zversion.h");
		rename("zversion.new", "zversion.h");
	} else
		unlink("zversion.new");

	return 0;	
}

int main(int argc, char *argv[])
{
	int i, s1, s2, err = 0;
	Byte array[97];

	((void)argc);
	((void)argv);

#ifdef __unix__
	uname(&utsname);
#endif

	if (NUMVARS != VARSNUM) {
		printf("Mismatch in NUMVARS and VARNUM %d:%d\n",
		       NUMVARS, VARSNUM);
		err = 1;
	}

	s1 = sizeof(key_label) / sizeof(char *);
	if (N_KEYS != NUM_SPECIAL || s1 != NUM_SPECIAL) {
		printf("Mismatch N_KEYS %d NUM_SPECIAL %d labels %d\n",
		       N_KEYS, NUM_SPECIAL, s1);
		err = 1;
	}

	if (NUM_SPECIAL > 32) {
		printf("Too many special keys\n");
		err = 1;
	} else {
		unsigned long mask = 0;

		for (i = 0; i < NUM_SPECIAL; ++i)
			mask = (mask << 1) | 1;
		if (mask != KEY_MASK) {
			printf("Mismatch mask %08lx and %08lx\n",
			       mask, (unsigned long)KEY_MASK);
			err = 1;
		}
	}

	/* Spot check keys array */
	if (sizeof(Keys) != NUMKEYS) {
		printf("Problems with Keys array\n");
		err = 1;
	} else {
		memset(array, ZINSERT, sizeof(array));
		array[0] = ZUNDO;
		array[96] = ZDELETE_PREVIOUS_CHAR;
		if (memcmp(array, Keys + 31, sizeof(array))) {
			printf("Problems with Keys array 1\n");
			err = 1;
		}

		if (Keys[CX('1')] != ZONE_WINDOW ||
		    Keys[CX('=')] != ZPOSITION ||
		    Keys[CX('O')] != ZNEXT_WINDOW ||
		    Keys[CX('Z')] != ZZAP_TO_CHAR ||
		    Keys[CX(127)] != ZDELETE_PREVIOUS_WORD) {
			printf("Problems with Keys array 2\n");
			err = 1;
		}

		if (Keys[M('A')] != ZAGAIN ||
		    Keys[M('Z')] != ZSAVE_AND_EXIT ||
		    Keys[TC_UP] != ZPREVIOUS_LINE ||
		    Keys[TC_F12] != ZREVERT_FILE ||
		    Keys[M(127)] != ZDELETE_PREVIOUS_WORD) {
			printf("Problems with Keys array 3\n");
			err = 1;
		}
	}

	/* check sizes of various stuff */
	if (NUMFUNCS >= 256) {
		printf("Cnames[].fnum is a byte. Too many functions.\n");
		err = 1;
	}
	s1 = sizeof(Cnames) / sizeof(struct cnames);
	s2 = (sizeof(Cmds) / sizeof(void *) / 2) - 1;
	if (s1 != NUMFUNCS || s2 != NUMFUNCS) {
		printf("Cnames: %d Cmds: %d NUMFUNCS: %d\n", s1, s2, NUMFUNCS);
		exit(1); /* stop since the loop below might segfault */
	}

	/* validate the Cnames array the best we can */
	for (s1 = 1; s1 < NUMFUNCS; ++s1) {
		if (strcasecmp(Cnames[s1].name, Cnames[s1 - 1].name) <= 0) {
			printf("Problem: (%d) %s and %s\n",
			    s1, Cnames[s1 - 1].name, Cnames[s1].name);
			err = 1;
		}
		if (strlen(Cnames[s1].name) > (size_t)30) {
			printf("%s too long\n", Cnames[s1].name);
			err = 1;
		}
	}

	/* validate the Vars array */
	for (s1 = 1; s1 < NUMVARS; ++s1)
		if (strcasecmp(Vars[s1].vname, Vars[s1 - 1].vname) <= 0) {
			printf("Problem: (%d) %s and %s\n",
			    s1, Vars[s1 - 1].vname, Vars[s1].vname);
			err = 1;
		}

	/* getplete structs must be aligned */
	if (sizeof(struct cnames) % sizeof(char *)) {
		printf("struct cnames not aligned [%d/%d]\n",
		       (int)sizeof(struct cnames), (int)sizeof(char *));
		err = 1;
	}
	if (sizeof(struct avar) % sizeof(char *)) {
		printf("struct avar not aligned [%d/%d]\n",
		       (int)sizeof(struct avar), (int)sizeof(char *));
		err = 1;
	}

	if (err == 0)
		err = build_zversion_h();

		/* Freebsd does not support sed -i */
#if ZLIB
	if (noinclude(argc, argv, "zlib.h")) {
		puts("Cound not find zlib.h... disabling zlib support.");
		puts("Maybe set ZLIBINC in Makefile?");
		system("sed 's:^#define ZLIB://#define ZLIB:' config.h > config.h.new");
		system("sed 's/^LIBS += -lz/#LIBS += -lz/' Makefile > Makefile.new");
		rename("config.h.new", "config.h");
		rename("Makefile.new", "Makefile");
	}
#endif
#if SPELL
	if (noinclude(argc, argv, "aspell.h") ||
	    noinclude(argc, argv, "dlfcn.h")) {
		puts("Cound not find aspell.h and/or dlfcn.h..."
		     "disabling spell support.");
		puts("Maybe set ASPELLINC in Makefile?");
		system("sed 's:^#define SPELL://#define SPELL:' config.h > config.h.new");
		system("sed 's/^LIBS += -ldl/#LIBS += -ldl/' Makefile > Makefile.new");
		rename("config.h.new", "config.h");
		rename("Makefile.new", "Makefile");
	} else {
		if (strcmp(utsname.sysname, "FreeBSD") == 0) {
			/* FreeBSD does not need -ldl */
			system("sed 's/^LIBS += -ldl/#LIBS += -ldl/' Makefile > Makefile.new");
			rename("Makefile.new", "Makefile");
		}
	}
#endif

	return err;
}
