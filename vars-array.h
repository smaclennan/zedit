/* NOTE: Spaces not allowed in variable names!!! */
#include "vars.h"

struct avar Vars[] = {
	{ "add-nl",		FLAG,		{1},
	  "If this flag variable is set, the editor will insure that "
	  "all saved files end in a Newline. The default is on."
	},
	{ "backup",		FLAG,		{0},
	  "If this flag variable is set, a backup file is created when "
	  "a file is written. The default is off."
	},
	{ "c-extends",		STRING,		{0},
	  "This variable defines up to 8 extensions that turn on C "
	  "Mode. The extensions are seperated with the colon (:)."
	},
	{ "c-tabs",		DECIMAL,	{8},
	  "A tab is a single character that is displayed as one or more "
	  "spaces on the screen. This variable defines the number of "
	  "spaces displayed per tab in C mode buffers, even if the buffer "
	  "is in View mode. The default is 8 spaces. See also Tabs."
	},
	{ "comments",		FLAG,		{1},
#if !COMMENTBOLD || TERMCAP
	  "Note: Comment bolding disabled in this version of Zedit.\n\n"
#endif
	  "If set then Zedit tries to bold (display in red) "
	  "comments in C and shell mode buffers. Enabled by default. "
#if TERMINFO
	  "\n\nComment bolding may not be supported on all terminals."
#endif
	},
	{ "exact",		FLAG,		{1},
	  "If Exact is set, all searches and replaces are case "
	  "sensitive. This means that the search string 'string' will "
	  "only match 'string', not 'STRING', or 'String'. The default "
	  "is on."
	},
	{ "expand-paths",	FLAG,		{1},
	  "If ExpandPaths is set, all file names are expanded to a full "
	  "path. For example, the file name '../file.c' might be "
	  "expanded to '/home/sam/dir/file.c'. ~ and $ prefixed paths "
	  "are always expanded.  The default is on."
	},
	{ "fill-ch-width",	DECIMAL,	{72},
	  "This is the column at which the Text mode commands wrap the "
	  "right margin. It is also used by the Center Line command. "
	  "The default is 72."
	},
	{ "grep",		STRING,		{0},
	  "The command to use for the Zedit grep command. The default "
	  "is `grep -n'."
	},
	{ "kill-line",		FLAG,		{1},
	  "If this variable is set, the 'Delete to End of Line' command "
	  "will delete the entire line. If at the start of line, it "
	  "will delete the line including the NL. The default is on."
	},
	{ "lines",		FLAG,		{0},
	  "This flag variable turns on a line:column indicator on the "
	  "mode line. The Lines feature can cause a lot of cursor "
	  "movement and is irritating on some slow terminals. The "
	  "default is off."
	},
	{ "make-cmd",		STRING,		{0},
	  "This is a string variable which contains the default string "
	  "for the Make command. The default is 'make'."
	},
	{ "margin",		DECIMAL,	{0},
	  "This is a decimal variable that defines the left margin for "
	  "Text mode and the Format Paragraph command. It does not "
	  "affect the first line of the paragraph. The default is 0. "
	},
	{ "match",		FLAG,		{1},
	  "This variable controls the C and Shell Mode matching "
	  "function. Zedit will match all ')' and ']' with their "
	  "matching '(' or '[' by moving the cursor to the matching "
	  "character temporarily. If there is no match, Zedit will "
	  "beep. Default on."
	},
	{ "normal",		FLAG,		{1},
	  "If on, sets the default buffer mode to Normal. If off, sets "
	  "the default buffer mode to Text. Some file extensions "
	  "override this variable. The default is on."
	},
	{ "sh-extends",		STRING,		{0},
	  "This variable defines up to 8 extensions that turn on Shell "
	  "Mode. The extensions are seperated with the colon (:)."
	},
	{ "silent",		FLAG,		{0},
	  "If set, turns the bell off. Default is off (bell on)."
	},
	{ "single-scroll",	FLAG,		{0},
	  "If SingleScroll is set, a Next Line command at the bottom of "
	  "the screen will scroll one line. If not set, the Next Line "
	  "will scroll a few lines. The default is off."
	},
	{ "space-tabs",		FLAG,		{0},
	  "The tab character is normally inserted into the buffer as a "
	  "single ASCII 9 character. If the SpaceTabs variable is set, "
	  "then a tab is inserted as spaces in the buffer. This flag "
	  "does not affect tabs already in the buffer or tabs inserted "
	  "in the PAW. The default is off."
	},
	{ "tabs",		DECIMAL,	{8},
	  "A tab is a single character that is displayed as one or more "
	  "spaces on the screen. This variable defines the number of "
	  "space characters displayed per tab in buffers that are not in "
	  "a program mode. The default is 8 spaces per tab."
	},
	{ "tagfile",		STRING,		{0},
	  "This specifies the tag file name used by the Find Tag "
	  "command. It is a full path name to the tag file. The "
	  "default is empty."
	},
	{ "t-extends",		STRING,		{0},
	  "This variable defines up to 8 extensions that turn on Text "
	  "Mode. The extensions are seperated with the colon (:). An "
	  "extension of '.' matches no extension (i.e. no '.' in file "
	  "name)."
	},
	{ "visible-bell",	FLAG,		{1},
	  "If this variable is set, the bell will flash the screen "
	  "rather than producing an audible tone. The default is on if "
	  "flashing the screen is supported."
	},
};
#define VARSIZE		sizeof(struct avar)
#define NUMVARS		((int)(sizeof(Vars) / VARSIZE))
