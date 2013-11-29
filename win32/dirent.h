#ifndef __DIRENT_H__
#define __DIRENT_H__

#ifndef WIN32
#error WIN32 only
#endif

struct dirent {
	char *d_name;
};

typedef struct DIR {
	HANDLE handle;
	WIN32_FIND_DATA data;
	struct dirent ent;
} DIR;

static __inline DIR *opendir(char *dirname)
{
	DIR *dir = calloc(1, sizeof(struct DIR));
	if (!dir)
		return NULL;

	char path[PATHMAX];
	snprintf(path, sizeof(path), "%s/*", dir);
	dir->handle = FindFirstFile(path, &dir->data);
	if (dir->handle == INVALID_HANDLE_VALUE) {
		free(dir);
		return NULL;
	}

	return dir;
}

static __inline struct dirent *readdir(DIR *dir)
{
	if (dir->ent.d_name == NULL) {
		/* we filled in data in FindFirstFile above */
		dir->ent.d_name = dir->data.cFileName;
		return &dir->ent;
	}

	if (FindNextFile(dir->handle, &dir->data))
		return &dir->ent;

	return NULL;
}

static __inline void closedir(DIR *dir)
{
	FindClose(dir->handle);
	free(dir);
}

#endif
