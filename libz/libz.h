/* Copyright (C) 1988-2026 Sean MacLennan */

#ifndef _libz_h
#define _libz_h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifdef UNSIGNED_BYTES
#define Byte unsigned char
#else
#define Byte char
#endif

/* strlcpy.c */
#if defined(__linux__) || defined(WIN32)
size_t strlcpy(char *dst, const char *src, size_t dstsize);
size_t strlcat(char *dst, const char *src, size_t dstsize);
#endif
int strconcat(char *str, int len, ...);

/* dbg.c */
const char *Dbgfname(const char *fname);
void Dbg(const char *fmt, ...);

#endif
