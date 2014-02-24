/* Copyright (C) 1991,1992,1993,1996,1997,1998,1999,2000,2001,2002,2003,2007,2010,2011
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This is from glibc 2.17 (Slackware 14.1) */

/* Enable GNU extensions in fnmatch.h.  */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE	1
#endif

#include <assert.h>
#include <errno.h>
#include <ctype.h>

# include <string.h>
# include <stdlib.h>

#include "./fnmatch.h"

#ifndef __GNUC__
static _inline void *mempcpy(void *dest, const void *src, size_t n)
{
	return (char *)memcpy(dest, src, n) + n;
}
#endif

/* We often have to test for FNM_FILE_NAME and FNM_PERIOD being both set.  */
#define NO_LEADING_PERIOD(flags)					\
	((flags & (FNM_FILE_NAME | FNM_PERIOD)) == (FNM_FILE_NAME | FNM_PERIOD))

/* Comment out all this code if we are using the GNU C Library, and are not
   actually compiling the library itself.  This code is part of the GNU C
   Library, but also included in many other GNU distributions.  Compiling
   and linking in this code is a waste when using the GNU C library
   (especially if it is a shared library).  Rather than having every GNU
   program understand `configure --with-gnu-libc' and omit the object files,
   it is simpler to just do this in the source for each such file.  */

# if defined STDC_HEADERS || !defined isascii
#  define ISASCII(c) 1
# else
#  define ISASCII(c) isascii(c)
# endif

# ifdef isblank
#  define ISBLANK(c) (ISASCII (c) && isblank (c))
# else
#  define ISBLANK(c) ((c) == ' ' || (c) == '\t')
# endif
# ifdef isgraph
#  define ISGRAPH(c) (ISASCII (c) && isgraph (c))
# else
#  define ISGRAPH(c) (ISASCII (c) && isprint (c) && !isspace (c))
# endif

# define ISPRINT(c) (ISASCII (c) && isprint (c))
# define ISDIGIT(c) (ISASCII (c) && isdigit (c))
# define ISALNUM(c) (ISASCII (c) && isalnum (c))
# define ISALPHA(c) (ISASCII (c) && isalpha (c))
# define ISCNTRL(c) (ISASCII (c) && iscntrl (c))
# define ISLOWER(c) (ISASCII (c) && islower (c))
# define ISPUNCT(c) (ISASCII (c) && ispunct (c))
# define ISSPACE(c) (ISASCII (c) && isspace (c))
# define ISUPPER(c) (ISASCII (c) && isupper (c))
# define ISXDIGIT(c) (ISASCII (c) && isxdigit (c))

# define STREQ(s1, s2) ((strcmp (s1, s2) == 0))


#  define CHAR_CLASS_MAX_LENGTH  6 /* Namely, `xdigit'.  */

#  define IS_CHAR_CLASS(string)					\
	(STREQ (string, "alpha") || STREQ (string, "upper")	\
	 || STREQ (string, "lower") || STREQ (string, "digit")	\
	 || STREQ (string, "alnum") || STREQ (string, "xdigit")	\
	 || STREQ (string, "space") || STREQ (string, "print")	\
	 || STREQ (string, "punct") || STREQ (string, "graph")	\
	 || STREQ (string, "cntrl") || STREQ (string, "blank"))


/* Note that this evaluates C many times.  */
#define FOLD(c) ((flags & FNM_CASEFOLD) && ISUPPER (c) ? tolower (c) : (c))
#define CHAR	char
#define UCHAR	unsigned char
#define INT	int
#define FCT	internal_fnmatch
#define EXT	ext_match
#define END	end_pattern
#define STRUCT	fnmatch_struct
#define L(CS)	CS
#define STRLEN(S) strlen (S)
#define STRCAT(D, S) strcat (D, S)
#define MEMCHR(S, C, N) memchr (S, C, N)
#define STRCAT(D, S) strcat (D, S)

/*****************************************************************************/
/* Start of fnmatch_loop.c */

struct STRUCT
{
	const CHAR *pattern;
	const CHAR *string;
	int no_leading_period;
};

/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, nonzero if not.  */
static int FCT(const CHAR *pattern, const CHAR *string,
	       const CHAR *string_end, int no_leading_period, int flags,
	       struct STRUCT *ends, size_t alloca_used);
static int EXT(INT opt, const CHAR *pattern, const CHAR *string,
	       const CHAR *string_end, int no_leading_period, int flags,
	       size_t alloca_used);
static const CHAR *END(const CHAR *patternp);

static int
FCT(pattern, string, string_end, no_leading_period, flags, ends, alloca_used)
const CHAR *pattern;
const CHAR *string;
const CHAR *string_end;
int no_leading_period;
int flags;
struct STRUCT *ends;
size_t alloca_used;
{
	register const CHAR *p = pattern, *n = string;
	register UCHAR c;

	while ((c = *p++) != L('\0'))
	{
		int new_no_leading_period = 0;
		c = FOLD(c);

		switch (c)
		{
		case L('?'):
			if ((flags & FNM_EXTMATCH) == 0 && *p == '(')
			{
				int res = EXT(c, p, n, string_end, no_leading_period,
					      flags, alloca_used);
				if (res != -1)
					return res;
			}

			if (n == string_end)
				return FNM_NOMATCH;
			else if (*n == L('/') && (flags & FNM_FILE_NAME))
				return FNM_NOMATCH;
			else if (*n == L('.') && no_leading_period)
				return FNM_NOMATCH;
			break;

		case L('\\'):
			if (!(flags & FNM_NOESCAPE))
			{
				c = *p++;
				if (c == L('\0'))
					/* Trailing \ loses.  */
					return FNM_NOMATCH;
				c = FOLD(c);
			}
			if (n == string_end || FOLD((UCHAR)*n) != c)
				return FNM_NOMATCH;
			break;

		case L('*'):
			if ((flags & FNM_EXTMATCH) == 0 && *p == '(')
			{
				int res = EXT(c, p, n, string_end, no_leading_period,
					      flags, alloca_used);
				if (res != -1)
					return res;
			}
			else if (ends != NULL)
			{
				ends->pattern = p - 1;
				ends->string = n;
				ends->no_leading_period = no_leading_period;
				return 0;
			}

			if (n != string_end && *n == L('.') && no_leading_period)
				return FNM_NOMATCH;

			for (c = *p++; c == L('?') || c == L('*'); c = *p++)
			{
				if (*p == L('(') && (flags & FNM_EXTMATCH) != 0)
				{
					const CHAR *endp = END(p);
					if (endp != p)
					{
						/* This is a pattern.  Skip over it.  */
						p = endp;
						continue;
					}
				}

				if (c == L('?'))
				{
					/* A ? needs to match one character.  */
					if (n == string_end)
						/* There isn't another character; no match.  */
						return FNM_NOMATCH;
					else if (*n == L('/')
						 && (flags & FNM_FILE_NAME) == 0)
						/* A slash does not match a wildcard under
						   FNM_FILE_NAME.  */
						return FNM_NOMATCH;
					else
						/* One character of the string is consumed in matching
						   this ? wildcard, so *??? won't match if there are
						   less than three characters.  */
						++n;
				}
			}

			if (c == L('\0'))
				/* The wildcard(s) is/are the last element of the pattern.
				   If the name is a file name and contains another slash
				   this means it cannot match, unless the FNM_LEADING_DIR
				   flag is set.  */
			{
				int result = (flags & FNM_FILE_NAME) == 0 ? 0 : FNM_NOMATCH;

				if (flags & FNM_FILE_NAME)
				{
					if (flags & FNM_LEADING_DIR)
						result = 0;
					else
					{
						if (MEMCHR(n, L('/'), string_end - n) == NULL)
							result = 0;
					}
				}

				return result;
			}
			else
			{
				const CHAR *endp;
				struct STRUCT end;

				end.pattern = NULL;
				endp = MEMCHR(n, (flags & FNM_FILE_NAME) ? L('/') : L('\0'),
					      string_end - n);
				if (endp == NULL)
					endp = string_end;

				if (c == L('[')
				    || ((flags & FNM_EXTMATCH)
					&& (c == L('@') || c == L('+') || c == L('!'))
					&& *p == L('(')))
				{
					int flags2 = ((flags & FNM_FILE_NAME)
						      ? flags : (flags & ~FNM_PERIOD));

					for (--p; n < endp; ++n, no_leading_period = 0)
						if (FCT(p, n, string_end, no_leading_period, flags2,
							&end, alloca_used) == 0)
							goto found;
				}
				else if (c == L('/') && (flags & FNM_FILE_NAME))
				{
					while (n < string_end && *n != L('/'))
						++n;
					if (n < string_end && *n == L('/')
					    && (FCT(p, n + 1, string_end, flags & FNM_PERIOD, flags,
						    NULL, alloca_used) == 0))
						return 0;
				}
				else
				{
					int flags2 = ((flags & FNM_FILE_NAME)
						      ? flags : (flags & ~FNM_PERIOD));

					if (c == L('\\') && !(flags & FNM_NOESCAPE))
						c = *p;
					c = FOLD(c);
					for (--p; n < endp; ++n, no_leading_period = 0)
						if (FOLD((UCHAR)*n) == c
						    && (FCT(p, n, string_end, no_leading_period, flags2,
							    &end, alloca_used) == 0))
						{
						found:
							if (end.pattern == NULL)
								return 0;
							break;
						}
					if (end.pattern != NULL)
					{
						p = end.pattern;
						n = end.string;
						no_leading_period = end.no_leading_period;
						continue;
					}
				}
			}

			/* If we come here no match is possible with the wildcard.  */
			return FNM_NOMATCH;

		case L('['):
		{
			/* Nonzero if the sense of the character class is inverted.  */
			const CHAR *p_init = p;
			const CHAR *n_init = n;
			register int not;
			CHAR cold;
			UCHAR fn;

			if (n == string_end)
				return FNM_NOMATCH;

			if (*n == L('.') && no_leading_period)
				return FNM_NOMATCH;

			if (*n == L('/') && (flags & FNM_FILE_NAME))
				/* `/' cannot be matched.  */
				return FNM_NOMATCH;

			not = (*p == L('!') || (*p == L('^')));
			if (not)
				++p;

			fn = FOLD((UCHAR)*n);

			c = *p++;
			for (;;)
			{
				if (!(flags & FNM_NOESCAPE) && c == L('\\'))
				{
					if (*p == L('\0'))
						return FNM_NOMATCH;
					c = FOLD((UCHAR)*p);
					++p;

					goto normal_bracket;
				}
				else if (c == L('[') && *p == L(':'))
				{
					/* Leave room for the null.  */
					CHAR str[CHAR_CLASS_MAX_LENGTH + 1];
					size_t c1 = 0;
					const CHAR *startp = p;

					for (;;)
					{
						if (c1 == CHAR_CLASS_MAX_LENGTH)
							/* The name is too long and therefore the pattern
							   is ill-formed.  */
							return FNM_NOMATCH;

						c = *++p;
						if (c == L(':') && p[1] == L(']'))
						{
							p += 2;
							break;
						}
						if (c < L('a') || c >= L('z'))
						{
							/* This cannot possibly be a character class name.
							   Match it as a normal range.  */
							p = startp;
							c = L('[');
							goto normal_bracket;
						}
						str[c1++] = c;
					}
					str[c1] = L('\0');

					if ((STREQ(str, L("alnum")) && ISALNUM((UCHAR)*n))
					    || (STREQ(str, L("alpha")) && ISALPHA((UCHAR)*n))
					    || (STREQ(str, L("blank")) && ISBLANK((UCHAR)*n))
					    || (STREQ(str, L("cntrl")) && ISCNTRL((UCHAR)*n))
					    || (STREQ(str, L("digit")) && ISDIGIT((UCHAR)*n))
					    || (STREQ(str, L("graph")) && ISGRAPH((UCHAR)*n))
					    || (STREQ(str, L("lower")) && ISLOWER((UCHAR)*n))
					    || (STREQ(str, L("print")) && ISPRINT((UCHAR)*n))
					    || (STREQ(str, L("punct")) && ISPUNCT((UCHAR)*n))
					    || (STREQ(str, L("space")) && ISSPACE((UCHAR)*n))
					    || (STREQ(str, L("upper")) && ISUPPER((UCHAR)*n))
					    || (STREQ(str, L("xdigit")) && ISXDIGIT((UCHAR)*n)))
						goto matched;
					c = *p++;
				}
				else if (c == L('\0'))
				{
					/* [ unterminated, treat as normal character.  */
					p = p_init;
					n = n_init;
					c = L('[');
					goto normal_match;
				}
				else
				{
					int is_range = 0;

					{
						c = FOLD(c);
					normal_bracket:

						/* We have to handling the symbols differently in
						   ranges since then the collation sequence is
						   important.  */
						is_range = (*p == L('-') && p[1] != L('\0')
							    && p[1] != L(']'));

						if (!is_range && c == fn)
							goto matched;

						/* This is needed if we goto normal_bracket; from
						   outside of is_seqval's scope.  */
						// SAM is_seqval = 0;
						cold = c;
						c = *p++;
					}

					if (c == L('-') && *p != L(']'))
					{
						/* We use a boring value comparison of the character
						   values.  This is better than comparing using
						   `strcoll' since the latter would have surprising
						   and sometimes fatal consequences.  */
						UCHAR cend = *p++;

						if (!(flags & FNM_NOESCAPE) && cend == L('\\'))
							cend = *p++;
						if (cend == L('\0'))
							return FNM_NOMATCH;

						/* It is a range.  */
						if (cold <= fn && fn <= cend)
							goto matched;

						c = *p++;
					}
				}

				if (c == L(']'))
					break;
			}

			if (!not)
				return FNM_NOMATCH;
			break;

		matched:
			/* Skip the rest of the [...] that already matched.  */
			do
			{
			ignore_next:
				c = *p++;

				if (c == L('\0'))
					/* [... (unterminated) loses.  */
					return FNM_NOMATCH;

				if (!(flags & FNM_NOESCAPE) && c == L('\\'))
				{
					if (*p == L('\0'))
						return FNM_NOMATCH;
					/* XXX 1003.2d11 is unclear if this is right.  */
					++p;
				}
				else if (c == L('[') && *p == L(':'))
				{
					int c1 = 0;
					const CHAR *startp = p;

					while (1)
					{
						c = *++p;
						if (++c1 == CHAR_CLASS_MAX_LENGTH)
							return FNM_NOMATCH;

						if (*p == L(':') && p[1] == L(']'))
							break;

						if (c < L('a') || c >= L('z'))
						{
							p = startp;
							goto ignore_next;
						}
					}
					p += 2;
					c = *p++;
				}
				else if (c == L('[') && *p == L('='))
				{
					c = *++p;
					if (c == L('\0'))
						return FNM_NOMATCH;
					c = *++p;
					if (c != L('=') || p[1] != L(']'))
						return FNM_NOMATCH;
					p += 2;
					c = *p++;
				}
				else if (c == L('[') && *p == L('.'))
				{
					++p;
					while (1)
					{
						c = *++p;
						if (c == '\0')
							return FNM_NOMATCH;

						if (*p == L('.') && p[1] == L(']'))
							break;
					}
					p += 2;
					c = *p++;
				}
			} while (c != L(']'));
			if (not)
				return FNM_NOMATCH;
		}
		break;

		case L('+'):
		case L('@'):
		case L('!'):
			if ((flags & FNM_EXTMATCH) == 0 && *p == '(')
			{
				int res = EXT(c, p, n, string_end, no_leading_period, flags,
					      alloca_used);
				if (res != -1)
					return res;
			}
			goto normal_match;

		case L('/'):
			if (NO_LEADING_PERIOD(flags))
			{
				if (n == string_end || c != (UCHAR)*n)
					return FNM_NOMATCH;

				new_no_leading_period = 1;
				break;
			}
			/* FALLTHROUGH */
		default:
		normal_match :
			if (n == string_end || c != FOLD((UCHAR)*n))
				return FNM_NOMATCH;
		}

		no_leading_period = new_no_leading_period;
		++n;
	}

	if (n == string_end)
		return 0;

	if ((flags & FNM_LEADING_DIR) && n != string_end && *n == L('/'))
		/* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
		return 0;

	return FNM_NOMATCH;
}


static const CHAR *
END(const CHAR *pattern)
{
	const CHAR *p = pattern;

	while (1)
		if (*++p == L('\0'))
			/* This is an invalid pattern.  */
			return pattern;
		else if (*p == L('['))
		{
			/* Handle brackets special.  */
			/* Skip the not sign.  We have to recognize it because of a possibly
			   following ']'.  */
			if (*++p == L('!') || (*p == L('^')))
				++p;
			/* A leading ']' is recognized as such.  */
			if (*p == L(']'))
				++p;
			/* Skip over all characters of the list.  */
			while (*p != L(']'))
				if (*p++ == L('\0'))
					/* This is no valid pattern.  */
					return pattern;
		}
		else if ((*p == L('?') || *p == L('*') || *p == L('+') || *p == L('@')
			  || *p == L('!')) && p[1] == L('('))
			p = END(p + 1);
		else if (*p == L(')'))
			break;

	return p + 1;
}


static int
EXT(INT opt, const CHAR *pattern, const CHAR *string, const CHAR *string_end,
    int no_leading_period, int flags, size_t alloca_used)
{
	const CHAR *startp;
	int level;
	struct patternlist
	{
		struct patternlist *next;
		CHAR malloced;
		CHAR str[1]; /* Turbo C does not allow 0 */
	} *list = NULL;
	struct patternlist **lastp = &list;
	size_t pattern_len = STRLEN(pattern);
	int any_malloced = 0;
	const CHAR *p;
	const CHAR *rs;
	int retval = 0;

	/* Parse the pattern.  Store the individual parts in the list.  */
	level = 0;
	for (startp = p = pattern + 1; level >= 0; ++p)
		if (*p == L('\0'))
		{
			/* This is an invalid pattern.  */
			retval = -1;
			goto out;
		}
		else if (*p == L('['))
		{
			/* Handle brackets special.  */
			/* Skip the not sign.  We have to recognize it because of a possibly
			   following ']'.  */
			if (*++p == L('!') || (*p == L('^')))
				++p;
			/* A leading ']' is recognized as such.  */
			if (*p == L(']'))
				++p;
			/* Skip over all characters of the list.  */
			while (*p != L(']'))
				if (*p++ == L('\0'))
				{
					/* This is no valid pattern.  */
					retval = -1;
					goto out;
				}
		}
		else if ((*p == L('?') || *p == L('*') || *p == L('+') || *p == L('@')
			  || *p == L('!')) && p[1] == L('('))
			/* Remember the nesting level.  */
			++level;
		else if (*p == L(')'))
		{
			if (level-- == 0)
			{
				/* This means we found the end of the pattern.  */
#define NEW_PATTERN							\
				struct patternlist *newp;		\
				size_t slen = (opt == L('?') || opt == L('@') \
					       ? pattern_len : (p - startp + 1)); \
				slen = sizeof (struct patternlist) + (slen * sizeof (CHAR)); \
				newp = malloc(slen);			\
				if (newp == NULL)			\
				{					\
					retval = -2;			\
					goto out;			\
				}					\
				any_malloced = 1;			\
				newp->next = NULL;			\
				newp->malloced = 1;			\
				*((CHAR *)mempcpy(newp->str, startp, p - startp)) = L('\0'); \
				*lastp = newp;				\
				lastp = &newp->next
				NEW_PATTERN;
			}
		}
		else if (*p == L('|'))
		{
			if (level == 0)
			{
				NEW_PATTERN;
				startp = p + 1;
			}
		}
	assert(list != NULL);
	assert(p[-1] == L(')'));
#undef NEW_PATTERN

	switch (opt)
	{
	case L('*'):
		if (FCT(p, string, string_end, no_leading_period, flags, NULL,
			alloca_used) == 0)
			goto success;
		/* FALLTHROUGH */

	case L('+'):
		do
		{
			for (rs = string; rs <= string_end; ++rs)
				/* First match the prefix with the current pattern with the
				   current pattern.  */
				if (FCT(list->str, string, rs, no_leading_period,
					flags & FNM_FILE_NAME ? flags : flags & ~FNM_PERIOD,
					NULL, alloca_used) == 0
				    /* This was successful.  Now match the rest with the rest
				       of the pattern.  */
				    && (FCT(p, rs, string_end,
					    rs == string
					    ? no_leading_period
					    : rs[-1] == '/' && NO_LEADING_PERIOD(flags) ? 1 : 0,
					    flags & FNM_FILE_NAME
					    ? flags : flags & ~FNM_PERIOD, NULL, alloca_used) == 0
					/* This didn't work.  Try the whole pattern.  */
					|| (rs != string
					    && FCT(pattern - 1, rs, string_end,
						   rs == string
						   ? no_leading_period
						   : (rs[-1] == '/' && NO_LEADING_PERIOD(flags)
						      ? 1 : 0),
						   flags & FNM_FILE_NAME
						   ? flags : flags & ~FNM_PERIOD, NULL,
						   alloca_used) == 0)))
					/* It worked.  Signal success.  */
					goto success;
		} while ((list = list->next) != NULL);

		/* None of the patterns lead to a match.  */
		retval = FNM_NOMATCH;
		break;

	case L('?'):
		if (FCT(p, string, string_end, no_leading_period, flags, NULL,
			alloca_used) == 0)
			goto success;
		/* FALLTHROUGH */

	case L('@'):
		do
			/* I cannot believe it but `strcat' is actually acceptable
			   here.  Match the entire string with the prefix from the
			   pattern list and the rest of the pattern following the
			   pattern list.  */
			if (FCT(STRCAT(list->str, p), string, string_end,
				no_leading_period,
				flags & FNM_FILE_NAME ? flags : flags & ~FNM_PERIOD,
				NULL, alloca_used) == 0)
				/* It worked.  Signal success.  */
				goto success;
		while ((list = list->next) != NULL);

		/* None of the patterns lead to a match.  */
		retval = FNM_NOMATCH;
		break;

	case L('!'):
		for (rs = string; rs <= string_end; ++rs)
		{
			struct patternlist *runp;

			for (runp = list; runp != NULL; runp = runp->next)
				if (FCT(runp->str, string, rs, no_leading_period,
					flags & FNM_FILE_NAME ? flags : flags & ~FNM_PERIOD,
					NULL, alloca_used) == 0)
					break;

			/* If none of the patterns matched see whether the rest does.  */
			if (runp == NULL
			    && (FCT(p, rs, string_end,
				    rs == string
				    ? no_leading_period
				    : rs[-1] == '/' && NO_LEADING_PERIOD(flags) ? 1 : 0,
				    flags & FNM_FILE_NAME ? flags : flags & ~FNM_PERIOD,
				    NULL, alloca_used) == 0))
				/* This is successful.  */
				goto success;
		}

		/* None of the patterns together with the rest of the pattern
		   lead to a match.  */
		retval = FNM_NOMATCH;
		break;

	default:
		assert(!"Invalid extended matching operator");
		retval = -1;
		break;
	}

success:
out :
	if (any_malloced)
		while (list != NULL)
		{
			struct patternlist *old = list;
			list = list->next;
			if (old->malloced)
				free(old);
		}

	return retval;
}

int
fnmatch (pattern, string, flags)
const char *pattern;
const char *string;
int flags;
{
	return FCT (pattern, string, string + strlen (string),
		    flags & FNM_PERIOD, flags, NULL, 0);
}
