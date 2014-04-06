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
#define strlwr _strlwr
#define unlink _unlink
#define getcwd _getcwd

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

#endif /* WIN32 */

#ifdef DOS
#include <time.h>

#define inline
#define _inline

typedef int bool;
#define true  1
#define false 0

#define vsnprintf(a, b, c, d) vsprintf(a, c, d)
#define strcasecmp stricmp
#define strncasecmp strnicmp

#define usleep(us) delay((us) / 1000)

#ifdef DOS_EMS
extern int ems_pages;

void ems_init(void);
void ems_free(void);
bool ems_newpage(struct page *page);
void ems_freepage(struct page *page);
void ems_makecur(struct page *page, bool curmodf);
void ems_pagesplit(struct page *newp, bool curmodf);
#endif /* DOS_EMS */

#endif /* DOS */

/* COMMON */

extern int optind;
extern char *optarg;
int getopt(int argc, char *argv[], const char *optstring);

#define F_OK 0
#define R_OK 4
#define W_OK 2

typedef int pid_t;

#endif
