/* Copyright (C) 1988-2018 Sean MacLennan */

#include "z.h"

/** @addtogroup zedit
 * @{
 */

static char Tagfile[PATHMAX];

/* SAM Currently works for functions only */
/* Leaves the point at the char offset */
#define TAG_REGEX "[^a-zA-Z0-9_]%s(.*[0-9][0-9]*,"

#define BOOKMARKS	16			/* number of book marks */
static struct mark *Bookmrks[BOOKMARKS];	/* stack of book marks */
static int  Bookmark = -1;			/* current book mark */
static int  Lastbook = -1;			/* last bookmark */

static int set_bookmark(void)
{
	Bookmark = (Bookmark + 1) & (BOOKMARKS - 1);
	if (Bookmark > Lastbook) {
		Bookmrks[Bookmark] = bcremark(Bbuff);
		if (!Bookmrks[Bookmark])
			return -1;
		Lastbook = Bookmark;
	} else
		bmrktopnt(Bbuff, Bookmrks[Bookmark]);
	return Bookmark;
}

static int get_tagfile(void)
{
	if (Argp) {
		Arg = 0;
		return getfname("Tagfile: ", Tagfile);
	}

	if (access(Tagfile, F_OK) == 0)
		return 0;

	if (Curbuff->fname) {
		char *p;
		strlcpy(Tagfile, Curbuff->fname, sizeof(Tagfile) - 4);
		p = strrchr(Tagfile, '/');
		if (p) {
			++p;
			strcpy(p, "TAGS");
			if (access(Tagfile, F_OK) == 0)
				return 0;
		}
	}

	strconcat(Tagfile, sizeof(Tagfile), zgetcwd(Tagfile, sizeof(Tagfile)),
		  "TAGS", NULL);
	if (access(Tagfile, F_OK) == 0)
		return 0;

	*Tagfile = '\0';
	return getfname("Tagfile: ", Tagfile);
}

static bool tagfile_modified(struct zbuff *buff)
{
	struct stat sbuf;

	if (strcmp(buff->fname, Tagfile))
		return true;

	if (stat(Tagfile, &sbuf) || sbuf.st_mtime > buff->mtime)
		return true;

	return false;
}

/* May change Curbuff */
static struct zbuff *read_tagfile(void)
{
	struct zbuff *buff;

	if (get_tagfile())
		return NULL;

	/* Check if we can reuse an existing tag buffer */
	buff = cfindbuff(TAGBUFF);
	if (buff && tagfile_modified(buff) == false)
		return buff;

	buff = cmakebuff(TAGBUFF, Tagfile);
	if (!buff)
		return NULL;

	if (zreadfile(Tagfile))
		return NULL;

	return buff;
}

static bool find_tag(char *word)
{
	char path[PATHMAX], regstr[STRMAX], *p;
	struct regexp re;
	struct zbuff *buff, *save = Curbuff;
	int offset;

	buff = read_tagfile();
	zswitchto(save);
	if (!buff)
		return false;

	strfmt(regstr, sizeof(regstr), TAG_REGEX, word);
	if (re_compile(&re, regstr, 0)) {
		putpaw("regcomp failed");
		return false;
	}

	zswitchto(buff);
	btostart(Bbuff);
	if (!re_step(Bbuff, &re, NULL))
		goto failed;

	offset = batoi();

	if (!bcrsearch(Bbuff, 014)) /* C-L */
		goto failed;
	bmove(Bbuff, 2);

	strcpy(path, Tagfile);
	p = strrchr(path, '/');
	if (p)
		++p;
	else
		p = path + strlen(path);
	getbword(p, sizeof(path), bistoken);

	zswitchto(save);
	set_bookmark();

	if (findfile(path)) {
		boffset(Bbuff, offset);
		redisplay();
		re_free(&re);
		return true;
	}

failed:
	putpaw("No match.");
	zswitchto(save);
	re_free(&re);
	return false;
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

void Zset_bookmark(void)
{
	Arg = 0;
	set_bookmark();
	putpaw("Book Mark %d Set", Bookmark + 1);
}

void Znext_bookmark(void)
{
	if (Bookmark < 0) {
		putpaw("No bookmarks set.");
		return;
	}

	if (Bookmrks[Bookmark]->mbuff != Bbuff) {
		set_last_bufname(Curbuff);
		wgoto(Bookmrks[Bookmark]->mbuff);
	}
	bpnttomrk(Bbuff, Bookmrks[Bookmark]);
	Curwdo->modeflags = INVALID;
	putpaw("Book Mark %d", Bookmark + 1);
	if (--Bookmark < 0)
		Bookmark = Lastbook;
}
/* @} */
