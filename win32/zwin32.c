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
		}
		else if (++optind >= argc)
			return '?';
		else
			optarg = argv[optind];
	}

	return arg;
}

char *gethomedir(void)
{
	static char home[PATHMAX];
	char *homedrive = getenv("HOMEDRIVE");
	char *homepath = getenv("HOMEPATH");

	if (!homedrive || !homepath)
		return NULL;

	snprintf(home, sizeof(home), "%s%s", homedrive, homepath);

	char *p = home;
	while ((p = strchr(p, '\\'))) *p = '/';

	return home;
}

void zgetcwd(char *dir, int len)
{
	_getcwd(dir, len);

	char *p = dir;
	while ((p = strchr(p, '\\'))) *p = '/';
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

	while ((to = strchr(to, '\\')))
		*to = '/';

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