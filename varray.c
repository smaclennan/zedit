#include "z.h"

#define V(n) {(char *)n}

struct avar Vars[] = {
	{ "backup",		V_FLAG,		V(0),
	  "If set, a backup file is created when a file is written."
	},
	{ "c-extends",		V_STRING,	V(".c.h.cpp.cc.cxx.y.l"),
	  "This variable defines the extensions that turn on C mode."
	},
	{ "c-tabs",		V_DECIMAL,	V(4),
	  "This variable defines the number of spaces displayed per tab in "
	  "C mode buffers. See also tabs."
	},
	{ "comments",		V_FLAG,		V(1),
	  "If set then Zedit tries to bold (display in red) comments in C "
	  "and shell mode buffers."
	},
	{ "exact",		V_FLAG,		V(1),
	  "If set, all searches and replaces are case sensitive. This means "
	  "that the search string 'string' will only match 'string', not "
	  "'STRING', or 'String'."
	},
	{ "fill-ch-width",	V_DECIMAL,	V(72),
	  "This is the column at which the text mode commands wrap the "
	  "right margin."
	},
	{ "make-cmd",		V_STRING,	V("make "),
	  "The command to use for the make command."
	},
	{ "normal",		V_FLAG,		V(1),
	  "If set, the default buffer mode is Normal. If off, "
	  "the default buffer mode is Text. Some file extensions "
	  "override this variable."
	},
	{ "sh-extends",		V_STRING,	V(".sh.csh.el"),
	  "This variable defines the extensions that turn on shell mode. "
	  "Note that a '#!/bin/sh' type line at the start of the file can "
	  "also turn on shell mode."
	},
	{ "sh-tabs",		V_DECIMAL,	V(4),
	  "This variable defines the number of spaces displayed per indent in "
	  "shell mode buffers. If `space-tabs' is set, the indent will always be "
	  "spaces. Else, the indent will be a combination of physical tabs and "
	  "spaces."
	},
	{ "space-tabs",		V_FLAG,		V(0),
	  "The tab character is normally inserted into the buffer as a "
	  "single ASCII 9 character. If the space-tabs variable is set, "
	  "then a tab is inserted as spaces in the buffer. This flag "
	  "does not affect tabs already in the buffer or tabs inserted "
	  "in the PAW."
	},
	{ "t-extends",		V_STRING,	V(".txt"),
	  "This variable defines the extensions that turn on text mode."
	},
	{ "tabs",		V_DECIMAL,	V(8),
	  "A tab is a single character that is displayed as one or more "
	  "spaces on the screen. This variable defines the number of "
	  "space characters displayed per physical tab.\n\n"
	  "You generally don't want to change this. See `c-tabs' and "
	  "`sh-tabs'."
	},
};
