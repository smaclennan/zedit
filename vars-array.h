/* NOTE: Spaces not allowed in variable names!!! */
#include "vars.h"

struct avar Vars[] = {
	{ "add-nl",		FLAG,		{1} },
	{ "backup",		FLAG },
	{ "c-extends",		STRING },
	{ "c-tabs",		DECIMAL,	{8} },
	{ "exact",		FLAG,		{1} },
	{ "expand-paths",	FLAG,		{1} },
	{ "fill-ch-width",	DECIMAL,	{72} },
	{ "flow-control",	FLAG },
	{ "grep",		STRING },
	{ "kill-line",		FLAG,		{1} },
	{ "lines",		FLAG },
	{ "make-cmd",		STRING },
	{ "margin",		DECIMAL },
	{ "match",		DECIMAL,	{1} },
	{ "normal",		FLAG,		{1} },
	{ "sh-extends",		STRING },
	{ "silent",		FLAG },
	{ "single-scroll",	FLAG },
	{ "space-tabs",		FLAG },
	{ "tabs",		DECIMAL,	{8} },
	{ "tagfile",		STRING },
	{ "t-extends",		STRING },
	{ "visible-bell",	FLAG,		{1} },
};
#define VARSIZE		sizeof(struct avar)
#define NUMVARS		((int)(sizeof(Vars) / VARSIZE))
