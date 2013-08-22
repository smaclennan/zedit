#ifndef VARSTR
#include "vars.h"
#endif

#define DEFAULT(n)	{n}

/* NOTE: Spaces not allowed in variable names!!! */
struct avar Vars[] = {
	{ "add-nl",		FLAG,		DEFAULT(1) },
	{ "asm-char",		STRING,		DEFAULT(0) },
	{ "asm-extends",	STRING,		DEFAULT(0) },
	{ "backup",		FLAG,		DEFAULT(0) },
	{ "c-extends",		STRING,		DEFAULT(0) },
	{ "c-tabs",		DECIMAL,	DEFAULT(8) },
	{ "date-format",	STRING,		DEFAULT(0) },
	{ "exact",		FLAG,		DEFAULT(1) },
	{ "expand-paths",	FLAG,		DEFAULT(1) },
	{ "fill-ch-width",	DECIMAL,	DEFAULT(72) },
	{ "flow-control",	FLAG,		DEFAULT(0) },
	{ "grep",		STRING,		DEFAULT(0) },
	{ "kill-line",		FLAG,		DEFAULT(1) },
	{ "lines",		FLAG,		DEFAULT(1) },
	{ "make-cmd",		STRING,		DEFAULT(0) },
	{ "margin",		DECIMAL,	DEFAULT(0) },
	{ "match",		DECIMAL,	DEFAULT(1) },
	{ "normal",		FLAG,		DEFAULT(1) },
	{ "overwrite",		FLAG,		DEFAULT(0) },
	{ "save-on-exit",	FLAG,		DEFAULT(0) },
	{ "silent",		FLAG,		DEFAULT(0) },
	{ "single-scroll",	FLAG,		DEFAULT(0) },
	{ "space-tabs",		FLAG,		DEFAULT(0) },
	{ "tabs",		DECIMAL,	DEFAULT(8) },
	{ "tagfile",		STRING,		DEFAULT(0) },
	{ "tcl-extends",	STRING,		DEFAULT(0) },
	{ "t-extends",		STRING,		DEFAULT(0) },
	{ "visible-bell",	FLAG,		DEFAULT(1) },
};
#define VARSIZE		sizeof(struct avar)
#define NUMVARS		((int)(sizeof(Vars) / VARSIZE))
