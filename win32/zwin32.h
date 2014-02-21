#ifndef __ZWIN32_H__
#define __ZWIN32_H__

#ifdef WIN32

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

#endif

#ifdef DOS
#include <time.h>

typedef int bool;
#define true  1
#define false 0

typedef int pid_t;

#define vsnprintf(a, b, c, d) vsprintf(a, c, d)
#define strcasecmp stricmp
#define strncasecmp strnicmp

#define usleep(us) delay((us) / 1000)
#endif

/* COMMON */

extern int optind;
extern char *optarg;
int getopt(int argc, char *argv[], const char *optstring);

#define F_OK 0
#define R_OK 4
#define W_OK 2

#endif
