/* NOTE: Spaces not allowed in variable names!!! */
#include "vars.h"

struct avar Vars[] = {
	{ "add-nl",		FLAG,		{1},
	  "If this flag variable is set, the editor will insure that "
	  "all saved files end in a Newline."
	},
	{ "backup",		FLAG,		{0},
	  "If this flag variable is set, a backup file is created when "
	  "a file is written."
	},
	{ "bell",		FLAG,		{1},
	  "If this variable is set, the bell will flash the screen."
	},
	{ "c-extends",		STRING,		.u.str = ".c:.h:.cpp:.cc:.cxx:.y:.l",
	  "This variable defines up to 8 extensions that turn on C "
	  "Mode. The extensions are seperated with the colon (:)."
	},
	{ "c-tabs",		DECIMAL,	{8},
	  "A tab is a single character that is displayed as one or more "
	  "spaces on the screen. This variable defines the number of "
	  "spaces displayed per tab in C mode buffers, even if the buffer "
	  "is in View mode. See also tabs."
	},
	{ "comments",		FLAG,		{1},
	  "If set then Zedit tries to bold (display in red) "
	  "comments in C and shell mode buffers."
	},
	{ "exact",		FLAG,		{1},
	  "If Exact is set, all searches and replaces are case "
	  "sensitive. This means that the search string 'string' will "
	  "only match 'string', not 'STRING', or 'String'."
	},
	{ "fill-ch-width",	DECIMAL,	{72},
	  "This is the column at which the Text mode commands wrap the "
	  "right margin. It is also used by the Center Line command. "
	},
	{ "grep",		STRING,		.u.str = "grep -n",
	  "The command to use for the Zedit grep command. The default "
	  "is `grep -n'."
	},
	{ "lines",		FLAG,		{0},
	  "This flag variable turns on a line:column indicator on the "
	  "mode line. The Lines feature can cause a lot of cursor "
	  "movement and is irritating on some slow terminals."
	},
	{ "make-cmd",		STRING,		.u.str = "make ",
	  "This is a string variable which contains the default string "
	  "for the Make command. The default is 'make'."
	},
	{ "normal",		FLAG,		{1},
	  "If on, sets the default buffer mode to Normal. If off, sets "
	  "the default buffer mode to Text. Some file extensions "
	  "override this variable."
	},
	{ "sh-extends",		STRING,		.u.str = ".sh:.csh:.el",
	  "This variable defines up to 8 extensions that turn on Shell "
	  "Mode. The extensions are seperated with the colon (:)."
	},
	{ "space-tabs",		FLAG,		{0},
	  "The tab character is normally inserted into the buffer as a "
	  "single ASCII 9 character. If the SpaceTabs variable is set, "
	  "then a tab is inserted as spaces in the buffer. This flag "
	  "does not affect tabs already in the buffer or tabs inserted "
	  "in the PAW."
	},
	{ "tabs",		DECIMAL,	{8},
	  "A tab is a single character that is displayed as one or more "
	  "spaces on the screen. This variable defines the number of "
	  "space characters displayed per tab in buffers that are not in "
	  "a program mode."
	},
	{ "t-extends",		STRING,		.u.str = "txt",
	  "This variable defines up to 8 extensions that turn on Text "
	  "Mode. The extensions are seperated with the colon (:). An "
	  "extension of '.' matches no extension (i.e. no '.' in file "
	  "name)."
	},
};
