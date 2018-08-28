/* reg.h - buffer regular expressions
 * Copyright (C) 1988-2018 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* We support three implementations of regular expressions, all of
 * which provide the same API to the application.
 *
 * 1. The default is to use the standard Unix regex.h functions. This
 * gives you egrep style regular expressions.
 *
 * 2. If HAVE_PCRE is defined, we use the Perl style regular expressions.
 *
 * 3. If BUILTIN_REG is defined, we use some old code from the ed
 * editor that provided a simple version of regular expressions. This
 * gives you grep style regular expressions.
 */

#ifndef _reg_h
#define _reg_h

#ifdef BUILTIN_REG
#define ESIZE		256			/* reg exp buffer size */

typedef struct regex {
	uint8_t ep[ESIZE];
	int circf;
} regexp_t;

#define REG_EXTENDED	0
#define REG_ICASE		0
#define REG_NOSUB		0
#define REG_NEWLINE		0
#else
# ifdef HAVE_PCRE
#  include "pcreposix.h"
# else
#  include <regex.h>
# endif

typedef struct regexp {
	regex_t re;
	int circf;
} regexp_t;
#endif

int re_compile(regexp_t *re, const char *regex, int cflags);
int re_step(struct buff *buff, regexp_t *re, struct mark *REstart);
void re_error(int errcode, const regexp_t *preg, char *errbuf, int errbuf_size);
void re_free(regexp_t *re);
int lookingat(struct buff *buff, const char *str);
int _lookingat(struct buff *buff, regexp_t *re);

#endif
