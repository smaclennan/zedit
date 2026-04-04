/* Copyright (C) 1988-2018 Sean MacLennan */

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

/** @addtogroup buffer
 * @{
 */

#ifdef BUILTIN_REG
#define ESIZE		256			/* reg exp buffer size */

struct regexp {
	uint8_t ep[ESIZE];
	int circf;
};

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

struct regexp {
	regex_t re;
	int circf;
};
#endif

int re_compile(struct regexp *re, const char *regex, int cflags);
int re_step(struct buff *buff, struct regexp *re, struct mark *REstart);
void re_error(int errcode, const struct regexp *preg,
	      char *errbuf, int errbuf_size);
void re_free(struct regexp *re);
int lookingat(struct buff *buff, const char *str);
int _lookingat(struct buff *buff, struct regexp *re);
/* @} */

#endif

/*
 * Local Variables:
 * my-checkpatch-ignores: "SPDX_LICENSE_TAG,NEW_TYPEDEFS"
 * End:
 */
