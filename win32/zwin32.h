#ifndef __ZWIN32_H__
#define __ZWIN32_H__

#include <Windows.h>
#include <io.h>
#include <direct.h>

/* WIN32 sure likes underscores */
#define inline __inline

#define snprintf _snprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strdup _strdup
#define unlink _unlink
#define getcwd zgetcwd

#define open _open
#define read _read
#define write _write
#define close _close
#define access _access
#define umask _umask
#define chdir _chdir
#define putenv _putenv
#define popen _popen
#define pclose _pclose

void zgetcwd(char *path, int len);

#define F_OK 0
#define R_OK 4
#define W_OK 2

extern HANDLE hstdin, hstdout;

extern int optind;
extern char *optarg;
int getopt(int argc, char *argv[], const char *optstring);

struct dirent {
	char *d_name;
};

typedef struct DIR {
	HANDLE handle;
	WIN32_FIND_DATAA data;
	struct dirent ent;
} DIR;

DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dir);
void closedir(DIR *dir);

#endif
