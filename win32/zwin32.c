#include "z.h"

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
