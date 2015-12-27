#ifndef __ZWIN32_H__
#define __ZWIN32_H__

#include <Windows.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>

/* WIN32 sure likes underscores */
#define snprintf _snprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strdup _strdup
#define strlwr _strlwr
#define unlink _unlink
#define getcwd _getcwd

#define creat(a, b) _creat(a, _S_IWRITE)
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

extern HANDLE hstdin, hstdout;

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

extern int optind;
extern char *optarg;
int getopt(int argc, char *argv[], const char *optstring);

#define F_OK 0
#define R_OK 4
#define W_OK 2

typedef int pid_t;

#endif
