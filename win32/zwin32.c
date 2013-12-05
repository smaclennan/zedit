#include "z.h"

int optind = 1;
char *optarg;

int getopt(int argc, char *argv[], const char *optstring)
{
	static char *argptr;
	char arg, *p;

	optarg = NULL;

	if (argptr && !*argptr) {
		++optind;
		argptr = NULL;
	}

	if (!argptr) {
		if (optind >= argc)
			return -1;

		argptr = argv[optind];
		if (*argptr++ != '-')
			return -1;
		if (strcmp(argptr, "-") == 0) {
			++optind;
			return -1;
		}
		if (*argptr == '\0')
			return -1;
	}

	arg = *argptr++;
	p = strchr(optstring, arg);
	if (p == NULL)
		return '?';

	if (*(p + 1) == ':') {
		if (*argptr) {
			optarg = argptr;
			argptr = NULL;
			++optind;
		} else if (++optind >= argc)
			return '?';
		else
			optarg = argv[optind];
	}

	return arg;
}

static __inline void psepfixup(char *path)
{
	while ((path = strchr(path, '\\')))
		*path = '/';
}

char *gethomedir(void)
{
	static char home[PATHMAX];
	char *homedrive = getenv("HOMEDRIVE");
	char *homepath = getenv("HOMEPATH");

	if (!homedrive || !homepath)
		return NULL;

	snprintf(home, sizeof(home), "%s%s", homedrive, homepath);
	psepfixup(home);

	return home;
}

void zgetcwd(char *dir, int len)
{
	_getcwd(dir, len);
	psepfixup(dir);
}

/* Fixup the pathname. 'to' and 'from' cannot overlap.
 * Currently just a trivial version.
 */

int pathfixup(char *to, char *from)
{
	/* If there is a drive letter... assume fully rooted */
	if (isalpha(*from) && *(from + 1) == ':')
		strcpy(to, from);
	/* Also assume if it starts with a slash it is rooted */
	else if (*from == '/' || *from == '\\')
		strcpy(to, from);
	else {
		getcwd(to, PATHMAX);
		strcat(to, "/");
		strcat(to, from);
	}

	psepfixup(to);

	return 0;
}

DIR *opendir(const char *dirname)
{
	DIR *dir = calloc(1, sizeof(struct DIR));
	if (!dir)
		return NULL;

	char path[PATHMAX];
	snprintf(path, sizeof(path), "%s/*", dirname);
	dir->handle = FindFirstFileA(path, &dir->data);
	if (dir->handle == INVALID_HANDLE_VALUE) {
		free(dir);
		return NULL;
	}

	return dir;
}

struct dirent *readdir(DIR *dir)
{
	if (dir->ent.d_name == NULL) {
		/* we filled in data in FindFirstFile above */
		dir->ent.d_name = dir->data.cFileName;
		return &dir->ent;
	}

	if (FindNextFileA(dir->handle, &dir->data))
		return &dir->ent;

	return NULL;
}

void closedir(DIR *dir)
{
	FindClose(dir->handle);
	free(dir);
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

void Zcmd_to_buffer(void)
{
	static char cmd[STRMAX + 1];
	struct wdo *save;
	int rc;

	Arg = 0;
	if (getarg("@ ", cmd, STRMAX) == 0) {
		save = Curwdo;
		do_chdir(Curbuff);
		if (wuseother(SHELLBUFF)) {
			putpaw("Please wait...");
			rc = pipetobuff(Curbuff, cmd);
			if (rc == 0) {
				message(Curbuff, cmd);
				btostart();
				putpaw("Done.");
			} else if (rc == -1)
				putpaw("Unable to execute.");
			else
				putpaw("Exit %d.", rc);
			Curbuff->bmodf = false;
			wswitchto(save);
		}
	}
}
