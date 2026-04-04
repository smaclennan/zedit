/* Copyright (C) 2016-2017 Sean MacLennan <seanm@seanm.ca> */

#ifndef __BWIN32_H__
#define __BWIN32_H__

#include <windows.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>

/* WIN32 sure likes underscores */
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strdup _strdup
#define strlwr _strlwr
#define unlink _unlink
#define getcwd _getcwd

/* VS 2017 doesn't seem to like _creat */
#define creat(a, b) _open(a, _O_CREAT | _O_TRUNC | _O_WRONLY, b)
#define open _open
#define read _read
#define write _write
#define lseek _lseek
#define close _close

#define access _access
#define umask _umask
#define chdir _chdir
#define putenv _putenv
#define popen _popen
#define pclose _pclose

#define inline _inline

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

/* This mask defines which events are sent to the winkbd_event_cb */
#define WINKBD_EVENT_MASK (ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)

/* Return non-zero if you don't want tgetkb to handle the event */
extern int (*winkbd_event_cb)(INPUT_RECORD *event);

#endif
