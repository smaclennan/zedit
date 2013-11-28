#include "z.h"

#ifndef WIN32
#include <regex.h>

static char Tagfile[PATHMAX];

/* SAM Currently works for functions only */
#define TAG_REGEX "[^a-zA-Z0-9_]%s\\([^0-9]+([0-9]+),([0-9]+)$"

static int get_tagfile(void)
{
	if (Argp) {
		Arg = 0;
		return getfname("Tagfile: ", Tagfile);
	}

	if (access(Tagfile, F_OK) == 0)
		return 0;

	if (Curbuff->fname) {
		strcpy(Tagfile, Curbuff->fname);
		char *p = strrchr(Tagfile, '/');
		if (p) {
			++p;
			strcpy(p, "TAGS");
			if (access(Tagfile, F_OK) == 0)
				return 0;
		}
	}

	*Tagfile = '\0';
	return getfname("Tagfile: ", Tagfile);
}

static bool find_tag(char *word)
{
	char fname[PATHMAX], line[PATHMAX], path[PATHMAX], *p;
	regex_t tagreg;
	regmatch_t pmatch[3];
	bool found = false;

	if (get_tagfile())
		return false;

	snprintf(line, sizeof(line), TAG_REGEX, word);
	if (regcomp(&tagreg, line, REG_EXTENDED | REG_NEWLINE)) {
		putpaw("regcomp failed");
		return false;
	}

	FILE *fp = fopen(Tagfile, "r");
	if (!fp) {
		putpaw("Unable to open tagfile.");
		goto done;
	}

	while (fgets(line, sizeof(line), fp))
		if (*line == 014) { /* C-L */
			if (fgets(line, sizeof(line), fp))
				strcpy(fname, line);
		} else if (regexec(&tagreg, line, 3, pmatch, 0) == 0) {
			int offset = strtol(line + pmatch[2].rm_so, NULL, 10);

			/* Isolate the filename */
			p = strchr(fname, ',');
			if (p)
				*p = '\0';
			strcpy(path, Tagfile);
			p = strrchr(path, '/');
			if (p) {
				++p;
				*p = '\0';
			}
			strcat(path, fname);

			set_bookmark(word);

			if (findfile(path)) {
				bgoto_char(offset);
				redisplay();
				found = true;
			}

			break;
		}

	if (!found)
		putpaw("%s not found.", word);

done:
	regfree(&tagreg);
	fclose(fp);
	return found;
}

void Ztag(void)
{
	char tag[STRMAX];

	*tag = '\0';
	if (getarg("Tag: ", tag, sizeof(tag)) == 0)
		find_tag(tag);
}

void Ztag_word(void)
{
	char tag[STRMAX];

	getbword(tag, sizeof(tag), bisword);
	find_tag(tag);
}
#else
void Ztag(void) { tbell(); }
void Ztag_word(void) { tbell(); }
#endif
