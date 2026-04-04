/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#define LIBZ_MAJOR 1
#define LIBZ_MINOR 0

/* \cond skip */
/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x)	#x
#define __stringify(x)	__stringify_1(x)

/* The marker is for `strings liblibz.a | fgrep Version' */
const char *libbuff_marker =
	"Version-" __stringify(LIBZ_MAJOR) "." __stringify(LIBZ_MINOR);

const char *libbuff_version =
	__stringify(LIBZ_MAJOR) "." __stringify(LIBZ_MINOR);
/* \endcond */
