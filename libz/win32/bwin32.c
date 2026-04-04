/* Copyright (C) 2016-2017 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

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

DIR *opendir(const char *dirname)
{
	DIR *dir = calloc(1, sizeof(struct DIR));
	if (!dir)
		return NULL;

	char path[MAX_PATH];
	strfmt(path, sizeof(path), "%s/*", dirname);
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

struct iovec {
	void *iov_base;
	size_t iov_len;
};

/* Not optimal... but should work */
int readv(int fd, const struct iovec *iov, int iovcnt)
{
	int i, n, n_read = 0;

	for (i = 0; i < iovcnt; ++i, ++iov)
		if (iov->iov_len) {
			n = read(fd, iov->iov_base, iov->iov_len);
			if (n < 0)
				return n;
			else if (n == 0)
				return n_read;
			n_read += n;
		}

	return n_read;
}

/* Not optimal... but should work */
int writev(int fd, const struct iovec *iov, int iovcnt)
{
	int i, n, n_wrote = 0;

	for (i = 0; i < iovcnt; ++i, ++iov)
		if (iov->iov_len) {
			n = write(fd, iov->iov_base, iov->iov_len);
			if (n < 0)
				return n;
			else if (n == 0)
				return n_wrote;
			n_wrote += n;
		}

	return n_wrote;
}
