/* This is old SUN BSD cruft */

#ifdef BSD
int access(char *path, int mode)
{
	int rc;
	struct stat s;

	if (stat(path, &s) == 0) {
		errno = EPERM;
		switch (mode) {
		case 0:
			return 0;
		case 2:
			if (s.st_uid == Me->pw_uid)
				rc = s.st_mode & 0200;
			else if (s.st_gid == Me->pw_gid)
				rc = s.st_mode & 020;
			else
				rc = s.st_mode & 02;
			return rc ? 0 : EOF;
		}
	}
	return EOF;
}
#endif

#ifdef BSD
	signal(SIGTSTP, SIG_DFL);		/* set signals so that we can */
	signal(SIGCONT, tinit);		/* suspend & restart Zedit */
#endif
