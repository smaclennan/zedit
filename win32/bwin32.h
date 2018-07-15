/* bwin32.h - generic windows header
 * Copyright (C) 2016-2017 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __BWIN32_H__
#define __BWIN32_H__

#include <windows.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

/* WIN32 sure likes underscores */
#ifndef snprintf
#define snprintf _snprintf
#endif
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

#endif
