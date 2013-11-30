#ifndef __ZWIN32_H__
#define __ZWIN32_H__

#include <Windows.h>
#include <io.h>
#include <direct.h>

#define inline __inline

#define snprintf _snprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strdup _strdup
#define unlink _unlink

#define open _open
#define read _read
#define write _write
#define close _close
#define access _access
#define umask _umask
#define putenv _putenv
#define getcwd _getcwd

#define F_OK 0
#define R_OK 4
#define W_OK 2

extern HANDLE hstdin, hstdout;
#endif
