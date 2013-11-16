#include "z.h"

struct cnames Cnames[] = {
	{"abort",			ZABORT,			H_OTHER,
	 "Aborts the current command. This is the only way to exit Universal\n"
	 "Arguments and String Arguments. It will also abort C-X and Meta\n"
	 "prefixes. Cannot abort the Quote command.\n"
	},
	{"again",			ZAGAIN,			H_SEARCH,
	 "Causes the last search to be executed again. If the mode Exact is set,\n"
	 "the search will be case sensitive. A Universal Argument causes the\n"
	 "search to look for an exact number matches of the string. If the number\n"
	 "cannot be matched, it will display the number actually matched in the\n"
	 "PAW and leave the Point where it was. If no search commands have been\n"
	 "executed, a Forward Search is executed.\n"
	},
	{"append-kill",			ZAPPEND_KILL,		H_DELETE,
	 "Sets the delete flag. The next delete command will append to the kill\n"
	 "buffer. A Universal Argument is ignored.\n"
	},
	{"arg",				ZARG,			H_OTHER,
	 "Many of the commands accept a Universal Argument to cause the command to\n"
	 "repeat or to modify the meaning of the command in some way. It reads\n"
	 "decimal digits and displays them in the PAW. The first non-digit is\n"
	 "processed as a command. The digits cannot be deleted since Delete is a\n"
	 "valid command that accepts a Universal Argument. If a mistake is made,\n"
	 "use the Abort Command.\n"
	},
	{"beginning-of-buffer",		ZBEGINNING_OF_BUFFER,	H_CURSOR,
	 "Moves the Point to the beginning of the current buffer. A Universal\n"
	 "Argument is ignored.\n"
	},
	{"beginning-of-line",		ZBEGINNING_OF_LINE,	H_CURSOR,
	 "Moves the Point to the beginning of the line or to the beginning of the\n"
	 "previous line. A Universal Argument causes the command to repeat.\n"
	},
	{"bind",			ZBIND,			H_BIND,
	 "Binds a function to a key. It first prompts, with command completion,\n"
	 "for the command to bind. It then prompts for the key to bind the command\n"
	 "to. The Meta and C-X prefix keys are entered, Bind prompts for another\n"
	 "key. The Abort command exits any prompt. The Quote command can be used\n"
	 "to rebind the Abort, Quote, C-X, and Meta commands. A Universal Argument\n"
	 "causes the Bind command to reset all the default arguments.  Keys can be\n"
	 "'unbound' by binding them to the NULL command.\n"
	},
	{"bound-to",			ZBOUND_TO,		H_BIND,
	 "Prompts for a command with command complete and gives a list of the keys\n"
	 "the command is bound to. A Universal Argument is ignored.\n"
	},
	{"c-indent",			ZC_INDENT,		H_MODE,
	 "Causes Newline characters to auto-indent to the current tab level. If a\n"
	 "Newline is hit after an open brace ({), the next line is indented an\n"
	 "extra tab stop. Bound to Newline in C mode. A Universal Argument causes\n"
	 "the command to repeat.\n"
	},
	{"c-insert",			ZC_INSERT,		H_SPECIAL,
	 "Tries to perform brace matching. When a close brace (}) is entered,\n"
	 "searches for a previous unmatched open brace ({). If it finds one, tries\n"
	 "to put the close brace in the same column as the column of the first\n"
	 "non-whitespace character in the matched line. If no match is found,\n"
	 "beeps. Bound to close brace in C mode. If bound to a character other than\n"
	 "a close brace, performs a Character Insert. A Universal Argument is\n"
	 "ignored.\n"
	},
	{"calc",			ZCALC,			H_HELP,
	 "This is a very simple integer calculator. It handles addition,\n"
	 "subtraction, multiplication, and division between\n"
	 "octal/decimal/hexadecimal numbers. If a number begins with a '0x' it is\n"
	 "assumed to be hexadecimal, if it begins with a '0' it is assumed to be\n"
	 "octal, all others are assumed to be decimal. The output is displayed in\n"
	 "decimal with hexadecimal in brackets '()'. Spaces and tabs are allowed\n"
	 "but are not necessary between the numbers and their operators. A Universal\n"
	 "Argument is ignored.\n"
	},
	{"capitalize-word",		ZCAPITALIZE_WORD,	H_DISP,
	 "In C mode, converts the character at the Point to uppercase and the rest\n"
	 "of the word to lowercase. In other modes, converts the first letter of\n"
	 "the current word to uppercase and the rest of the characters in the word\n"
	 "to lowercase. The Point is left at the end of the word. A Universal\n"
	 "Argument causes the command to repeat.\n"
	},
	{"center",			ZCENTER,		H_DISP,
	 "Centers the current line on the screen. It uses FillWidth to define the\n"
	 "right margin. A Universal Argument causes the command to repeat on the\n"
	 "next lines.\n"
	},
	{"cmd-to-buffer",		ZCMD_TO_BUFFER,		H_SHELL,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Prompts  for  a command and then executes the command in the shell. The\n"
	 "output is put in the '.shell' buffer. Any previous contents of the\n"
	 "'.shell' buffer are deleted. A Universal Argument is ignored.\n"
	},
	{"copy-region",			ZCOPY_REGION,		H_DELETE,
	 "Copies the region to the kill buffer. The kill buffer is overwritten\n"
	 "unless the the delete flag is set. See Append Kill command.\n"
	},
	{"copy-word",			ZCOPY_WORD,		H_DELETE,
	 "Copies the word the Point is on to the kill buffer. The kill buffer is\n"
	 "overwritten unless the the delete flag is set.In the PAW, the Copy Word\n"
	 "command takes the word the Point was on in the previously active window\n"
	 "and inserts it into the PAW.\n"
	},
	{"count",			ZCOUNT,			H_HELP,
	 "Count the number of lines, words, and characters in the region. A\n"
	 "Universal Argument counts the entire buffer.\n"
	},
	{"ctrl-x",			ZCTRL_X,		H_SPECIAL,
	 "Command prefix.\n"
	},
	{"cwd",				ZCWD,			H_OTHER,
	 "This command allows you to change Zedit's current working directory. All\n"
	 "files will be relative to the new working directory. The default is the\n"
	 "directory the editor was started in. A Universal Argument causes the\n"
	 "command to repeat.\n"
	},
	{"delete-blanks",		ZDELETE_BLANKS,		H_DELETE,
	 "Delete all the blank lines around the Point. The lines are not put in\n"
	 "the Kill Buffer. A Universal Argument causes the command to repeat,\n"
	 "which accomplishes nothing.\n"
	},
	{"delete-buffer",		ZDELETE_BUFFER,		H_BUFF,
	 "Deletes the current buffer and goes to a previous buffer. There must\n"
	 "always be at least one buffer. If the buffer has been modified, Zedit\n"
	 "asks to save it before deleting it. A Universal Argument prompts for the\n"
	 "buffer to delete.\n"
	},
	{"delete-char",			ZDELETE_CHAR,		H_DELETE,
	 "Deletes the character at the Point and leaves the Point on the next\n"
	 "character in the buffer. The character is not put in the Kill Buffer. A\n"
	 "Universal Argument causes the command to repeat.\n"
	},
	{"delete-line",			ZDELETE_LINE,		H_DELETE,
	 "Deletes the entire line, including the Newline, no matter where the\n"
	 "Point is in the line. The Point is left at the start of the next line.\n"
	 "The deleted line is put in the Kill Buffer. A Universal Argument causes\n"
	 "the command to repeat.\n"
	},
	{"delete-previous-char",	ZDELETE_PREVIOUS_CHAR,	H_DELETE,
	 "Deletes the character before the Point and leaves the Point in the same\n"
	 "place. The character is not put in the Kill Buffer. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
	{"delete-previous-word",	ZDELETE_PREVIOUS_WORD,	H_DELETE,
	 "Deletes the word to the left of the Point. The character the Point is on\n"
	 "is not deleted. The word is put in the Kill Buffer. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
	{"delete-region",		ZDELETE_REGION,		H_DELETE,
	 "Deletes the characters in the region. The deleted characters are put in\n"
	 "the Kill Buffer based on the delete flag. A Universal Argument is\n"
	 "ignored.\n"
	},
	{"delete-to-eol",		ZDELETE_TO_EOL,		H_DELETE,
	 "Deletes the characters from the Point to the end of the line. If the\n"
	 "Point is at the end of a line, the Newline character is deleted and the\n"
	 "next line is joined to the end of the current line. The characters\n"
	 "deleted are put in the Kill Buffer. A Universal Argument causes the\n"
	 "command to repeat.\n"
	},
	{"delete-window",		ZDELETE_WINDOW,		H_BUFF,
	 "Deletes the current window and places you in the other window. You\n"
	 "cannot delete the last window. A Universal Argument is ignored.\n"
	},
	{"delete-word",			ZDELETE_WORD,		H_DELETE,
	 "Deletes the word to the right of and including the Point. The word is\n"
	 "put in the Kill Buffer. A Universal Argument causes the command to\n"
	 "repeat.\n"
	},
	{"display-bindings",		ZDISPLAY_BINDINGS,	H_BIND,
	 "Inserts a list of the commands and their current bindings into the\n"
	 ".list buffer. It deletes the contents of the .list buffer. Keys\n"
	 "bound to NULL or Insert are not displayed. The PAW column displays\n"
	 "if the command is allowed in the PAW.\n"
	 "A Universal Argument prompts for an output file to put the list in.\n"
	},
	{"empty-buffer",		ZEMPTY_BUFFER,		H_BUFF,
	 "Deletes the entire contents of the current buffer. A Universal Arguments\n"
	 "is ignored.\n"
	},
	{"end-of-buffer",		ZEND_OF_BUFFER,		H_CURSOR,
	 "Moves the Point to the end of the buffer. A Universal Argument is\n"
	 "ignored.\n"
	},
	{"end-of-line",			ZEND_OF_LINE,		H_CURSOR,
	 "Moves the Point to the end of the line or to the end of the next line. A\n"
	 "Universal Argument causes the command to repeat.\n"
	},
	{"exit",			ZEXIT,			H_OTHER,
	 "Exits from the editor. It asks to save all modified buffers. A bang\n"
	 "(!) saves all remaining buffers. A Universal Argument causes all\n"
	 "modified buffers to be saved without prompting.\n"
	},
	{"fill-check",			ZFILL_CHECK,		H_SPECIAL,
	 "Checks if the current column is past the FillWidth column. If it is, the\n"
	 "words past or on the FillWidth column are wrapped. This gives some word\n"
	 "processing capability to the editor. Normally bound to the space bar and\n"
	 "Newline only in Text Mode. A Universal Argument has no meaning.\n"
	},
	{"fill-paragraph",		ZFILL_PARAGRAPH,	H_MODE,
	 "Uses the FillWidth to reformat the paragraph the Point is in. This is\n"
	 "useful if editing has messed up the right margin. A Universal Argument\n"
	 "reformats the next Arg paragraphs. A Universal Argument of 0 reformats\n"
	 "the entire buffer. When reformatting the entire buffer, hitting a\n"
	 "character will abort the reformat. Not allowed in program mode buffers.\n"
	},
	{"find-file",			ZFIND_FILE,		H_FILE,
	 "Prompts for a path name. If a buffer already exists with this path name,\n"
	 "that buffer is switched to. If no buffer is matched, a new buffer is\n"
	 "created and the file read into it. Supports file name completion. A\n"
	 "Universal Argument causes the command to repeat.\n"
	},
	{"global-re-search",		ZGLOBAL_RE_SEARCH,	H_SEARCH,
	 "This command is used to search for a regular expression in all the buffers.\n"
	 "It prompts for a search string and then starts searching at the start of\n"
	 "the first buffer. If a match is found, it stops and leaves the point at\n"
	 "the match. If Again is executed, it starts at the current Point and\n"
	 "searches forwards through the buffers. At the last buffer it stops and\n"
	 "puts the Point back where it started. A Universal Argument is ignored.\n"
	},
	{"global-search",		ZGLOBAL_SEARCH,		H_SEARCH,
	 "This command is used to search for a string in all the buffers. It\n"
	 "prompts for a search string and then starts searching at the start of\n"
	 "the first buffer. If a match is found, it stops and leaves the point at\n"
	 "the match. If Again is executed, it starts at the current Point and\n"
	 "searches forwards through the buffers. At the last buffer it stops and\n"
	 "puts the Point back where it started. A Universal Argument is ignored.\n"
	},
	{"goto-line",			ZGOTO_LINE,		H_CURSOR,
	 "Moves the point to the start of a given line. If there is a Universal\n"
	 "Argument, uses the argument, else prompts for the line number. If the\n"
	 "line is past the end of the buffer, moves the Point is left at the end\n"
	 "of the buffer.\n"
	},
	{"grep",			ZGREP,			H_SHELL,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Prompts for the completion of a grep command and then performs the\n"
	 "grep command in the *grep* buffer.\n"
	},
	{"grow-window",			ZGROW_WINDOW,		H_BUFF,
	 "Makes the active window one line bigger, and therefore the inactive\n"
	 "window one line smaller. A window cannot be less than three (3) lines.\n"
	 "If there is only one window, this command has no effect. A Universal\n"
	 "Argument causes the command to grow the window by that number of lines.\n"
	},
	{"help-function",		ZHELP_FUNCTION,		H_HELP,
	 "Displays help on any of the Zedit functions.\n"
	 "Prompts for the function with full completion.\n"
	},
	{"help-group",			ZHELP_GROUP,		H_HELP,
	 "The Zedit functions are grouped into categories. This command lets\n"
	 "you see the grouped functions.\n"
	},
	{"help-variable",		ZHELP_VARIABLE,		H_HELP,
	 "Displays help on any of the configurable variables.\n"
	 "Prompts for the variable with full completion.\n"
	},
	{"hex-output",			ZHEX_OUTPUT,		H_HELP,
	 "Displays the character at the Point as a hexadecimal number in the PAW.\n"
	 "A Universal Argument displays the next characters up to a\n"
	 "maximum of 25. The Point is moved forward by the argument characters.\n"
	},
	{"incremental-search",		ZINCREMENTAL_SEARCH,	H_SEARCH,
	 "Searches for the string after every character is entered in the PAW. The\n"
	 "Delete Previous Character command can be used to delete a character and\n"
	 "'back up' the search. The Newline character terminates the search. The\n"
	 "Abort command terminates the search and places the Point back where it\n"
	 "was. Any other commands terminates the search and performs that command.\n"
	 "A Universal Argument causes the command to repeat.\n"
	},
	{"indent",			ZINDENT,		H_DISP,
	 "Indents the marked region Universal Argument tab stops.\n"
	},
	{"insert",			ZINSERT,		H_SPECIAL,
	 "Normally bound to the printable characters, causes the character to be\n"
	 "inserted in the buffer. A Universal Argument causes the command to\n"
	 "repeat.\n"
	},
	{"insert-overwrite",		ZINSERT_OVERWRITE,	H_MODE,
	 "Toggles the current buffers minor mode between Insert, the default, and\n"
	 "overwrite modes. On the PC the cursor shape reflects the mode. Insert\n"
	 "mode has an underline cursor while overwrite mode has a block cursor. A\n"
	 "Universal Argument is ignored.\n"
	},
	{"join",			ZJOIN,			H_DELETE,
	 "Joins two lines. Performs the following Zedit commands:\n"
	 "End of Line, Delete Newline, Trim Whitespace, Insert space.\n"
	},
	{"key-binding",			ZKEY_BINDING,		H_BIND,
	 "Prompts for a key and displays the current command bound to the key in the\n"
	 "PAW. Handles C-X and M- prefixes. A Universal Argument is ignored.\n"
	},
	{"kill",			ZKILL,			H_SHELL,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Kills the current make. The command must wait for the make to die before\n"
	 "it can continue. Unix only.\n"
	},
	{"life",			ZLIFE,			H_SHELL,
	 "Destructively plays the game of Life on the current buffer.\n"
	 "  A space does the next iteration.\n"
	 "  A Return causes continuous iterations (with a slight delay).\n"
	 "  An Abort stops the game.\n"
	},
	{"list-buffers",		ZLIST_BUFFERS,		H_BUFF,
	 "Displays a list of the current buffers on the display.\n"
	},
	{"lowercase-region",		ZLOWERCASE_REGION,	H_DISP,
	 "Converts the Region to lowercase. Not allowed in program mode buffers. A\n"
	 "Universal Argument is ignored.\n"
	},
	{"lowercase-word",		ZLOWERCASE_WORD,	H_DISP,
	 "Converts the current word starting at the Point to lowercase. It leaves\n"
	 "the Point at the end of the word. A Universal Argument causes the\n"
	 "command to repeat.\n"
	},
	{"make",			ZMAKE,			H_SHELL,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "This command performs the command specified in the Make variable and\n"
	 "puts the output in the '.make' buffer. The command is usually 'make' and\n"
	 "is used in conjunction with the Next Error command. A Universal Argument\n"
	 "prompts for the command to execute.\n"
	},
	{"mark-paragraph",		ZMARK_PARAGRAPH,	H_DELETE,
	 "Sets the Mark to the start of the current paragraph and moves the Point\n"
	 "to the start of the next paragraph. A Universal Argument causes the\n"
	 "command to repeat.\n"
	},
	{"meta",			ZMETA,			H_SPECIAL,
	 "Command prefix.\n"
	},
	{"meta-x",			ZMETA_X,		H_OTHER,
	 "Prompts for a command to execute. Unbound commands may be executed in\n"
	 "this manner. Supports command completion. A '?' will show all the\n"
	 "possible matches. A Universal Argument is passed on to the selected\n"
	 "command.\n"
	},
	{"mode",			ZMODE,			H_MODE,
	 "Change the mode of the current buffer. Prompts (with command completion)\n"
	 "for the mode to change to.  A Universal Argument causes multiple\n"
	 "toggles.\n"
	},
	{"newline",			ZNEWLINE,		H_SPECIAL,
	 "The command normally bound to the Enter or Return key.\n"
	 "In overwrite mode, a NL goes to start of next line.\n"
	 "In insert mode, its just inserted.\n"
	 "A Universal Argument causes the command to repeat.\n"
	},
	{"next-bookmark",		ZNEXT_BOOKMARK,		H_CURSOR,
	 "Moves the Point to the last bookmark set in the bookmark ring. The\n"
	 "bookmark moved to is displayed in the echo window. A Universal Argument\n"
	 "in the range 1 to 10 corresponding to a set bookmark will go to the\n"
	 "bookmark.\n"
	},
	{"next-buffer",			ZNEXT_BUFFER,		H_BUFF,
	 "Switches to the next buffer in the buffer list. At the end of the list,\n"
	 "it switches to the first buffer in the list, i.e. treats the list like a\n"
	 "ring. A Universal Argument causes the command to repeat.\n"
	},
	{"next-char",			ZNEXT_CHAR,		H_CURSOR,
	 "Moves the Point forward one character. If the Point is at the end of a\n"
	 "line, it is moved to the start of the next line. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
	{"next-error",			ZNEXT_ERROR,		H_SHELL,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Used after a Make command to search for error messages in the '.make'\n"
	 "buffer. If an error is found, the Mark in the '.make' buffer is placed\n"
	 "at the start of the error message. The file containing the error is\n"
	 "loaded into a buffer using the Find File command and the Point is placed\n"
	 "at the error line.\n"
	 "If a Universal Argument is specified, Next Error tries to ignore warnings.\n"
	},
	{"next-line",			ZNEXT_LINE,		H_CURSOR,
	 "Moves the Point up one line in the buffer. It tries to maintain the same\n"
	 "column position. If the line is to short the, Point will be the placed\n"
	 "at the end of the line. Consecutive Previous/Next Line or Page commands\n"
	 "try to maintain the original column position. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
	{"next-page",			ZNEXT_PAGE,		H_CURSOR,
	 "Moves the Point down one page and tries to center the Point line in the\n"
	 "display. It tries to maintain the same column position. If the line is\n"
	 "to short the, Point will be the placed at the end of the line.\n"
	 "Consecutive Previous/Next Line or Page commands try to maintain the\n"
	 "original column position. A Universal Argument causes the command to\n"
	 "repeat.\n"
	},
	{"next-paragraph",		ZNEXT_PARAGRAPH,	H_CURSOR,
	 "Moves the Point to the start of the next paragraph. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
	{"next-window",			ZNEXT_WINDOW,		H_DISP,
	 "Goes to the next window on the screen. The windows are treated as a\n"
	 "circular list. A Universal Argument goes to the Nth next window.\n"
	},
	{"next-word",			ZNEXT_WORD,		H_CURSOR,
	 "Moves the Point to the start of the next word. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
	{"notimpl",			ZNOTIMPL,		H_BIND,
	 "Used to unbind a key. This is bound to all the unbound keys. A Universal\n"
	 "Argument does nothing Arg times.\n"
	},
	{"one-window",			ZONE_WINDOW,		H_BUFF,
	 "Makes the active window a full screen window. If there is only one\n"
	 "window, this command has no effect. A Universal Argument is ignored.\n"
	},
	{"open-line",			ZOPEN_LINE,		H_DISP,
	 "Inserts a Newline at the Point but leaves the Point in front of the\n"
	 "Newline. It is the same as typing a Newline and then a Previous\n"
	 "Character command. A Universal Argument causes the command to repeat.\n"
	},
	{"other-next-page",		ZOTHER_NEXT_PAGE,	H_BUFF,
	 "Performs a Next Page command in the bottom window or the top window if\n"
	 "you are in the bottom window. It leaves the Point where it is in the\n"
	 "active window. If there is only one window, this command has no effect.\n"
	 "A Universal Argument causes the command to page the Nth window.\n"
	},
	{"other-previous-page",		ZOTHER_PREVIOUS_PAGE,	H_BUFF,
	 "Performs a Previous Page command in the bottom window or the top window\n"
	 "if you are in the bottom window. It leaves the Point where it is in the\n"
	 "active window. If there is only one window, this command has no effect.\n"
	 "A Universal Argument pages the Nth window.\n"
	},
	{"out-to",			ZOUT_TO,		H_CURSOR,
	 "This command moves the Point to an absolute column position. If the line\n"
	 "is shorter than the specified column, it is padded with tabs and spaces\n"
	 "to the specified column. The command takes either a Universal Argument or\n"
	 "prompts for the column to go to.\n"
	},
	{"position",			ZPOSITION,		H_HELP,
	 "Displays the current Point position as a line, column, and byte offset\n"
	 "in the echo window. Also displays the length of the buffer. A Universal\n"
	 "Argument is ignored.\n"
	},
	{"previous-char",		ZPREVIOUS_CHAR,		H_CURSOR,
	 "Moves the Point back one character. If the Point is at the start of the\n"
	 "line, it is moved to the end of the previous line. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
	{"previous-line",		ZPREVIOUS_LINE,		H_CURSOR,
	 "Moves the Point up one line in the buffer. It tries to maintain the same\n"
	 "column position. If the line is to short, the Point will be placed at\n"
	 "the end of the line. Consecutive Previous/Next Line or Page commands try\n"
	 "to maintain the original column position. A Universal Argument causes\n"
	 "the command to repeat.\n"
	},
	{"previous-page",		ZPREVIOUS_PAGE,		H_CURSOR,
	 "Moves the Point up one page and tries to center the Point line in the\n"
	 "display. It tries to maintain the same column position. If the line is\n"
	 "to short, the Point will be placed at the end of the line. Consecutive\n"
	 "Previous/Next Line or Page commands try to maintain the original column\n"
	 "position. A Universal Argument causes the command to repeat.\n"
	},
	{"previous-paragraph",		ZPREVIOUS_PARAGRAPH,	H_CURSOR,
	 "Moves the Point to the start of the paragraph or to the start of the\n"
	 "previous paragraph. A Universal Argument causes the command to repeat.\n"
	},
	{"previous-window",		ZPREVIOUS_WINDOW,	H_DISP,
	 "Goes to the previous window on the screen. The windows are treated as a\n"
	 "circular list. A Universal Argument goes to the Nth previous window.\n"
	},
	{"previous-word",		ZPREVIOUS_WORD,		H_CURSOR,
	 "Moves the Point back to the start of a word or to the start of the\n"
	 "previous word. A Universal Argument causes the command to repeat.\n"
	},
	{"query-replace",		ZQUERY_REPLACE,		H_SEARCH,
	 "Prompts for a search string and a replacement string and searches from the\n"
	 "current Point looking for matches of the search string. If it finds a\n"
	 "match, it moves the Point to the match and waits for one of the\n"
	 "following input characters:\n"
	 "\n"
	 "  ,              Replace current match and confirm.\n"
	 "  space          Replace current match and continue.\n"
	 "  y              Replace current match and continue.\n"
	 "  .              Replace and exit.\n"
	 "  !              Global replace.\n"
	 "  ^              Goto previous match.\n"
	 "  Abort          Exit without replacing current match.\n"
	 "  Others         Continue without replacing current match.\n"
	 "\n"
	 "The Point is left at the position it was in before the Query Replace. If\n"
	 "the mode Exact is set, the search will be case sensitive. A Universal\n"
	 "Argument causes the replacement to be done globally in all the buffers.\n"
	},
	{"quote",			ZQUOTE,			H_OTHER,
	 "The Quote command is used to insert a character into a buffer or String\n"
	 "Argument that would normally be a command. The next character after the\n"
	 "Quote command is taken literally, unless the key is a valid hexadecimal\n"
	 "digit. If the key is a valid hexadecimal digit, it and the next key are\n"
	 "taken together as a hexadecimal number and the ASCII character\n"
	 "equivalent of this number is inserted into the buffer. If a key is not\n"
	 "hit within approximately one second, 'Quote:' is displayed in the echo\n"
	 "window. The Quote command does not work within a Universal Argument. A\n"
	 "Universal Argument inserts Arg characters.\n"
	},
	{"re-replace",			ZRE_REPLACE,		H_SEARCH,
	 "Works like the Query Replace command except that the search string is a\n"
	 "regular expression. The replacement string is a literal with two\n"
	 "exceptions. An '&' character causes the matched string to be placed in\n"
	 "the buffer. The escape character '\' can be used to turn off this\n"
	 "special meaning of '&'. Note that '\\' is required to put a real '\' in\n"
	 "the buffer. For each match, a prompt is made for the action to perform.\n"
	 "See Query Replace for a list of valid actions. A Universal\n"
	 "Argument causes the replacement to be done globally in all the buffers.\n"
	},
	{"re-search",			ZRE_SEARCH,		H_SEARCH,
	 "Asks for a regular expression search string and searches from the Point\n"
	 "forward for a match in the buffer. If a match is found, the Point is\n"
	 "moved to the start of the match. If the string is not found, then 'Not\n"
	 "Found' is displayed in the echo window and the Point is left where it\n"
	 "was. The search string is saved and subsequent search and replace\n"
	 "commands will default to this string. If the mode Exact is set, the\n"
	 "search will be case sensitive. A Universal Argument causes the search\n"
	 "to look for exactly Arg matches of the string.\n"
	},
	{"read-file",			ZREAD_FILE,		H_FILE,
	 "Prompts for a path name, with file name completion,  and inserts the\n"
	 "file into the current buffer before the Point. If there is a Universal\n"
	 "Argument, the current buffer is deleted, first asking to save the file\n"
	 "if it is modified, before reading in the new file. The buffers file name\n"
	 "is not changed, any writes will be to the old file name.\n"
	},
	{"redisplay",			ZREDISPLAY,		H_DISP,
	 "Repaints the entire screen. A Universal Argument causes the command to\n"
	 "repeat.\n"
	},
	{"replace",			ZREPLACE,		H_SEARCH,
	 "Prompts for a search string and a replacement string and searches from\n"
	 "the current Point looking for matches of the search string. If it finds\n"
	 "a match, it replaces the string and continues. The Point is left at the\n"
	 "position it was in before the Query Replace. If the mode Exact is set,\n"
	 "the search will be case sensitive. A Universal Argument causes the\n"
	 "replacement to be done globally in all the buffers.\n"
	},
	{"reverse-inc-search",		ZREVERSE_INC_SEARCH,	H_SEARCH,
	 "Searches backwards for the string after every character is entered\n"
	 "in the PAW. The Delete Previous Character command can be used to\n"
	 "delete a character and 'forward up' the search. The Newline\n"
	 "character terminates the search. The Abort command terminates the\n"
	 "search and places the Point back where it was. Any other command\n"
	 "terminates the search and performs the command. A Universal\n"
	 "Argument causes the command to repeat.\n"
	},
	{"reverse-search",		ZREVERSE_SEARCH,	H_SEARCH,
	 "Prompts for a search string and searches from the Point backward for a\n"
	 "match in the buffer. If a match is found, the Point is moved to the\n"
	 "start of the match. If the string is not found, then 'Not Found' is\n"
	 "displayed in the echo window and the Point is not moved. The search\n"
	 "string is saved and subsequent search and replace commands will default\n"
	 "to the last search string. If the mode Exact is set, the search will be\n"
	 "case sensitive. A Universal Argument causes the search to look for\n"
	 "exactly Arg matches of the String Argument.\n"
	},
	{"revert-file",			ZREVERT_FILE,		H_FILE,
	 "Rereads the current file. If the file has changed, asks to overwrite\n"
	 "changes. A Universal Argument causes multiple rereads. Mimics the vi 'e'\n"
	 "command.\n"
	},
	{"save-all-files",		ZSAVE_ALL_FILES,	H_FILE,
	 "Saves all modified buffers. A Universal Argument causes ALL files to be\n"
	 "saved, modified or not.\n"
	},
	{"save-and-exit",		ZSAVE_AND_EXIT,		H_FILE,
	 "An attempt to mimic the vi ZZ command. Does a save file on the\n"
	 "current buffer and then exits. A universal argument will save all\n"
	 "files.\n"
	},
	{"save-bindings",		ZSAVE_BINDINGS,		H_BIND,
	 "A key bound with the Bind command only stays in effect for the\n"
	 "duration of the edit session. The Save Bindings command is used to\n"
	 "permanently save any new bindings in the bindings file. The file is\n"
	 "saved in the users home directory. A Universal Argument causes Arg\n"
	 "saves!\n"
	},
	{"save-file",			ZSAVE_FILE,		H_FILE,
	 "Saves the current buffer to its file. If the current buffer has no path\n"
	 "name, a path name is prompted for (with file name completion). A\n"
	 "Universal Argument is ignored.\n"
	},
	{"scroll-down",			ZSCROLL_DOWN,		H_DISP,
	 "Scrolls the screen down one line. A Universal Argument causes the\n"
	 "command to scroll multiple lines.\n"
	},
	{"scroll-up",			ZSCROLL_UP,		H_DISP,
	 "Scrolls the screen up one line. A Universal Argument causes the\n"
	 "command to scroll multiple lines.\n"
	},
	{"search",			ZSEARCH,		H_SEARCH,
	 "Prompts for a search string and searches from the Point forward for a\n"
	 "match in the buffer. If a match is found, the Point is moved to the\n"
	 "start of the match. If the string is not found, then 'Not Found' is\n"
	 "displayed in the echo window and the Point is not moved. The search\n"
	 "string is saved and subsequent search or replace commands will default\n"
	 "to the last search string. If the mode Exact is set, the search will be\n"
	 "case sensitive. A Universal Argument causes the search to look for\n"
	 "exactly Arg matches.\n"
	},
	{"set-bookmark",		ZSET_BOOKMARK,		H_CURSOR,
	 "Places an invisible 'bookmark' at the Point. There are 10\n"
	 "bookmarks placed in a ring. The bookmark set is displayed in the PAW.\n"
	 "A Universal Argument causes the command to repeat.\n"
	},
	{"set-mark",			ZSET_MARK,		H_DELETE,
	 "Sets the Mark at the Point. A Universal Argument is ignored.\n"
	},
	{"set-variable",		ZSET_VARIABLE,		H_HELP,
	 "Allows any of the configurable variables to be set. Command completion\n"
	 "is supported. Prompts for the variable, then prompts for the new\n"
	 "setting. A Universal Argument causes numeric or flag variables to be\n"
	 "set to Arg, string variables ignore the Universal Argument.\n"
	},
	{"setenv",			ZSETENV,		H_OTHER,
	 "Performs a 'setenv' command on an environment variable. The variable will\n"
	 "keep the setting during the the Zedit session.\n"
	},
	{"show-config",			ZSHOW_CONFIG,		H_HELP,
	 "Show the current settings of all the Zedit variables in a\n"
	 "buffer. The buffer is suitable for use as a .config.z file and can\n"
	 "be saved to a file using Write File.\n"
	},
	{"shrink-window",		ZSHRINK_WINDOW,		H_BUFF,
	 "Shrinks the current window one line. A Universal Argument shrinks the\n"
	 "window Arg lines.\n"
	},
	{"size-window",			ZSIZE_WINDOW,		H_BUFF,
	 "Sets the window to Universal Argument lines.\n"
	},
	{"split-window",		ZSPLIT_WINDOW,		H_BUFF,
	 "Splits the current window into two windows. The same buffer is displayed in\n"
	 "both windows and the bottom window is made active. A Universal Argument\n"
	 "keeps splitting the current window.\n"
	},
	{"stats",			ZSTATS,			H_BUFF,
	 "Displays some simple stats about Zedit. "
#if UNDO
	 "The undo stat is for the current buffer, not a global count."
#endif
	},
	{"swap-chars",			ZSWAP_CHARS,		H_DISP,
	 "Swaps the character before the Point with the character at the\n"
	 "Point. It leaves the Point after the second character. Successive\n"
	 "commands will 'drag' the character that was before the Point towards the\n"
	 "end of the line. If the Point was at the end of a line, the two\n"
	 "characters before the Point are transposed. If the Point is at the\n"
	 "beginning of the buffer, the character at the Point and the character\n"
	 "after the Point are transposed. A Universal Argument causes the command\n"
	 "to repeat.\n"
	},
	{"swap-mark",			ZSWAP_MARK,		H_CURSOR,
	 "The Mark goes to the current position of the Point and the Point is set\n"
	 "where the Mark was. A Universal Argument is ignored.\n"
	},
	{"swap-words",			ZSWAP_WORDS,		H_DISP,
	 "Swaps the words before and after the Point. The Point is left after\n"
	 "the second word. Successive commands will 'drag' the word before the\n"
	 "Point towards the end of the line. If the Point is at the end of a line,\n"
	 "the last word on the line is transposed with the first word on the next\n"
	 "line. The command does nothing at the end of the buffer. If the Point is\n"
	 "in the middle of a word, the two halves of the word are transposed. A\n"
	 "Universal Argument causes the command to repeat.\n"
	},
	{"switch-to-buffer",		ZSWITCH_TO_BUFFER,	H_BUFF,
	 "Prompts for a buffer name to switch to with command completion.\n"
	 "The previous buffer is stored and displayed as a default. This allows\n"
	 "quick switching between two buffers. The Again command can be used to\n"
	 "scroll through the buffer list. A Universal Argument is ignored.\n"
	},
	{"tab",				ZTAB,			H_SPECIAL,
	 "Handles the tab key. A Universal Argument causes the command to repeat.\n"
	},
	{"toggle-case",			ZTOGGLE_CASE,		H_SEARCH,
	 "Switches the current buffers minor mode between case sensitive and case\n"
	 "insensitive for searches and replacements. Ignores the Universal\n"
	 "Argument.\n"
	},
	{"trim-white-space",		ZTRIM_WHITE_SPACE,	H_DELETE,
	 "Removes all spaces and tabs on both sides of the Point. The Point is\n"
	 "left on the character that is to the right of the deleted text. The\n"
	 "deleted characters are not put in the Kill Buffer. A Universal Argument\n"
	 "is ignored.\n"
	},
	{"undent",			ZUNDENT,		H_DISP,
	 "Removes Universal Argument tabs from the start of every line in the\n"
	 "region.\n"
	},
	{"undo",			ZUNDO,			H_OTHER,
#if !UNDO
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "EXPERIMENTAL: Undo the previous edit. There is a list of undos.\n"
	},
	{"unmodify",			ZUNMODIFY,		H_DELETE,
	 "Turns off the modified flag for the current buffer. Does not change the\n"
	 "buffer itself. A Universal Argument is ignored.\n"
	},
	{"uppercase-region",		ZUPPERCASE_REGION,	H_DISP,
	 "Convert the Region to uppercase. Not allowed in program mode buffers. A\n"
	 "Universal Argument is ignored.\n"
	},
	{"uppercase-word",		ZUPPERCASE_WORD,	H_DISP,
	 "Converts the current word starting at the Point to uppercase. It leaves\n"
	 "the Point at the end of the word. A Universal Argument causes the\n"
	 "command to repeat.\n"
	},
	{"view-line",			ZVIEW_LINE,		H_DISP,
	 "Makes the Point line the top line in the window. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
	{"write-file",			ZWRITE_FILE,		H_FILE,
	 "Prompts for a path name and writes out the current buffer to this file\n"
	 "name. Supports file name completion. It changes the path name of the\n"
	 "current buffer to the new path name. A Universal Argument causes only\n"
	 "the Region to be written and does not change the name of the buffer.\n"
	},
	{"yank",			ZYANK,			H_DELETE,
	 "Inserts the characters from the Kill Buffer before the Point. The\n"
	 "characters are inserted, even in overwrite mode. A Universal Argument\n"
	 "causes the command to repeat.\n"
	},
};
