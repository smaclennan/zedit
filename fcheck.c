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
#include "vars-array.h"

static unsigned Key_mask;
static int Key_shortcut;

#if ANSI
#include "ansi.c"
#elif TERMINFO
#include "terminfo.c"
#elif TERMCAP
#include "termcap.c"
#else
#error No-screen-driver
#endif

#include "funcs.c"

void Dbg(char *fmt, ...) {}

#if BUILTIN_DOCS
#include <dirent.h>

static char *cmd_docs[NUMFUNCS];
static char doc_buffer[4096];


static int c_filter(const struct dirent *ent)
{
	char *p = strrchr(ent->d_name, '.');
	if (p)
		return strcmp(p, ".c") == 0;
	return 0;
}

static void add_one(char *func, char *doc)
{
	char *p;
	int i;

	for (p = func; *p; ++p)
		if (*p == '_')
			*p = '-';

	for( i = 0; i < NUMFUNCS; ++i ) {
		if( strcmp(Cnames[i].name, func) == 0 )
		{	/* found it! */
			if (cmd_docs[i]) {
				fprintf(stderr, "Already has doc!");
				exit(1);
			}

			cmd_docs[i] = strdup(doc);
			if (!cmd_docs[i]) {
				fputs("Out of memory!\n", stderr);
			}
			return;
		}
	}

	fprintf(stderr, "%s not found in cnames\n", func);
	exit(1);
}

static void process_one_file(char *fname)
{
	FILE *fp = fopen(fname, "r");
	if (!fp) {
		perror(fname);
		exit(1);
	}

	char line[128], *p;
	while (fgets(line, sizeof(line), fp))
		if (strncmp(line, "/***", 4) == 0) {
			*doc_buffer = '\0';
			while (fgets(line, sizeof(line), fp))
				if (strncmp(line, " */", 3) == 0)
					break;
				else
					strcat(doc_buffer, line + 3);
			if (fgets(line, sizeof(line), fp)) {
				if (strncmp(line, "void Z", 6)) {
					fprintf(stderr, "Doc with no func in %s", fname);
					exit(1);
				}
				for (p = line + 6; *p && *p != '('; ++p) ;
				*p = '\0';
				add_one(line + 6, doc_buffer);
			}
		}

	fclose(fp);
}

static int process_c_files(void)
{
	struct dirent **namelist;
	int n = scandir(".", &namelist, c_filter, alphasort);
	if (n < 0) {
		perror("scandir");
		exit(1);
	}

	while (n--) {
		process_one_file(namelist[n]->d_name);
		free(namelist[n]);
	}
	free(namelist);

	return 0;
}

static int build_func_help(void)
{
	char func[40], *p, *p1;
	int i, need_quote = 1;
	int err = process_c_files();

	FILE *fp = fopen("func-help.c", "w");
	if (!fp) {
		perror("func-help.c");
		return -1;
	}

	for( i = 0; i < NUMFUNCS; ++i )
		if( cmd_docs[i] == NULL) {
			err = 1;
			fprintf( stderr, "%s not found\n", Cnames[i].name);
		} else {
			for (p = Cnames[i].name, p1 = func; *p; ++p, ++p1)
				if (*p == '-')
					*p1 = '_';
				else
					*p1 = *p;
			*p1 = '\0';

			fprintf(fp, "const char *h_%s =", func);
			for (p = cmd_docs[i]; *p; ++p)
				if (*p == '\n') {
					fprintf(fp, "\\n\"");
					need_quote = 1;
				} else {
					if (need_quote) {
						fputs("\n\"", fp);
						need_quote = 0;
					}
					if (*p == '"')
						fputc('\'', fp);
					else
						fputc(*p, fp);
				}
			fprintf(fp, ";\n\n");
		}

	fclose(fp);

	return err;
}
#endif /* BUILTIN_DOCS */

int main(int argc, char *argv[])
{
	int i, err = 0;

	if (NUMVARS != VARNUM) {
		printf("Mismatch in NUMVARS and VARNUM %d:%d\n",
		       NUMVARS, VARNUM);
		err = 1;
	}

	if (N_KEYS != NUM_SPECIAL) {
		printf("Mismatch N_KEYS %d NUMKEYS %d\n",
		       N_KEYS, NUM_SPECIAL);
		err = 1;
	}

	/* check sizes of various stuff */
	int s1 = sizeof(Cnames) / sizeof(struct cnames);
	int s2 = (sizeof(Cmds) / sizeof(void *) / 2) - 1;
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
		if (strncmp(Cnames[s1].name, "Top", 3) == 0) {
			printf("Zhelp() Top: %s\n", Cnames[s1].name);
			err = 1;
		}
	}

#if BUILTIN_DOCS
	if (build_func_help())
		err = 1;
#endif

	return err;
}
