#include "z.h"

#define V(n) {(char *)n}

struct avar Vars[] = {
	{ "add-nl",		V_FLAG,		V(1),
	  "If this flag variable is set, the editor will insure that "
	  "all saved files end in a Newline."
	},
	{ "backup",		V_FLAG,		V(0),
	  "If this flag variable is set, a backup file is created when "
	  "a file is written."
	},
	{ "bell",		V_FLAG,		V(1),
	  "If this flag variable is set, the bell will flash the screen."
	},
	{ "c-extends",		V_STRING,	V(".c.h.cpp.cc.cxx.y.l"),
	  "This variable defines the extensions that turn on C mode."
	},
	{ "c-tabs",		V_DECIMAL,	V(8),
	  "A tab is a single character that is displayed as one or more "
	  "spaces on the screen. This variable defines the number of "
	  "spaces displayed per tab in C mode buffers. See also tabs."
	},
	{ "comments",		V_FLAG,		V(1),
	  "If set then Zedit tries to bold (display in red) "
	  "comments in C and shell mode buffers."
	},
	{ "exact",		V_FLAG,		V(1),
	  "If Exact is set, all searches and replaces are case "
	  "sensitive. This means that the search string 'string' will "
	  "only match 'string', not 'STRING', or 'String'."
	},
	{ "fill-ch-width",	V_DECIMAL,	V(72),
	  "This is the column at which the Text mode commands wrap the "
	  "right margin. It is also used by the Center Line command. "
	},
	{ "grep",		V_STRING,	V("grep -n"),
	  "The command to use for the Zedit grep command."
	},
	{ "lines",		V_FLAG,		V(0),
	  "This flag variable turns on a line:column indicator on the "
	  "mode line. The Lines feature can cause a lot of cursor "
	  "movement and is irritating on some slow terminals."
	},
	{ "make-cmd",		V_STRING,	V("make "),
	  "This is a string variable which contains the default string "
	  "for the Make command."
	},
	{ "normal",		V_FLAG,		V(1),
	  "If on, sets the default buffer mode to Normal. If off, sets "
	  "the default buffer mode to Text. Some file extensions "
	  "override this variable."
	},
	{ "sh-extends",		V_STRING,	V(".sh.csh.el"),
	  "This variable defines the extensions that turn on shell mode. "
	  "Note that a '#!/bin/sh' type line at the start of the file can "
	  "also turn on shell mode."
	},
	{ "sh-tabs",		V_DECIMAL,	V(4),
	  "This variable defines the number of spaces displayed per tab "
	  "in shell mode buffers. See also tabs."
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
	  "space characters displayed per tab in buffers that are not in "
	  "a program mode."
	},
};
