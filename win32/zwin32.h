#ifndef __ZWIN32_H__
#define __ZWIN32_H__

#include <Windows.h>

#define inline __inline

#define snprintf _snprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strcpy _strcpy
#define strcat _strcat
#define strlen _strlen

#define F_OK 0
#define R_OK 0
#define W_OK 0

#endif
