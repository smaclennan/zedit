#include "z.h"

/* For now it is a char. May be expanded later. */
#define AN '+'	/* Arg Normal */
#define AI '-'	/* Arg Ignored */

struct cnames Cnames[] = {
	{"abort",			ZABORT,			AI,
	 "Aborts the current command. This is the only way to exit Universal "
	 "and String Arguments. It will also abort C-X and Meta prefixes."
	},
	{"again",			ZAGAIN,			AN,
	 "Causes the last search to be executed again. If no search commands "
	 "have been executed, a search command is executed."
	},
	{"append-kill",			ZAPPEND_KILL,		AI,
	 "Sets the delete flag. The next delete command will append to the "
	 "kill buffer."
	},
	{"arg",				ZARG,			0,
	 "Many of the commands accept a Universal Argument to cause the "
	 "command to repeat or to modify the meaning of the command in some "
	 "way. It reads decimal digits and displays them in the PAW. The "
	 "first non-digit is processed as a command. The digits cannot be "
	 "deleted since Delete is a valid command that accepts a Universal "
	 "Argument. If a mistake is made, use the Abort Command."
	},
	{"beginning-of-buffer",		ZBEGINNING_OF_BUFFER,	AI,
	 "Moves the Point to the beginning of the current buffer."
	},
	{"beginning-of-line",		ZBEGINNING_OF_LINE,	AN,
	 "Moves the Point to the beginning of the line or to the beginning of "
	 "the previous line."
	},
	{"c-indent",			ZC_INDENT,		AN,
	 "Causes Newline characters to auto-indent to the current tab "
	 "level. If a Newline is hit after an open brace ({), the next line "
	 "is indented an extra tab stop. Bound to Newline in C mode."
	},
	{"c-insert",			ZC_INSERT,		AI,
	 "Tries to perform brace matching. When a close brace (}) is "
	 "entered, searches for a previous unmatched open brace ({). If it "
	 "finds one, tries to put the close brace in the same column as the "
	 "column of the first non-whitespace character in the matched "
	 "line. If no match is found, beeps. Bound to close brace in C "
	 "mode. If bound to a character other than a close brace, performs a "
	 "Character Insert."
	},
	{"calc",			ZCALC,			AI,
	 "This is a very simple integer calculator. It handles addition, "
	 "subtraction, multiplication, and division between "
	 "octal/decimal/hexadecimal numbers. If a number begins with a '0x' "
	 "it is assumed to be hexadecimal, if it begins with a '0' it is "
	 "assumed to be octal, all others are assumed to be decimal. The "
	 "output is displayed in decimal with hexadecimal in brackets "
	 "'()'. Spaces and tabs are allowed but are not necessary between "
	 "the numbers and their operators."
	},
	{"capitalize-word",		ZCAPITALIZE_WORD,	AN,
	 "Converts the first letter of the current word to uppercase and "
	 "the rest of the characters in the word to lowercase. The Point "
	 "is left at the end of the word."
	},
	{"center",			ZCENTER,		AN,
	 "Centers the current line on the screen. It uses fill-width to define "
	 "the right margin."
	},
	{"cmd-to-buffer",		ZCMD_TO_BUFFER,		AI,
	 "Prompts for a command and then executes the command in the "
	 "shell. The output is put in the " SHELLBUFF " buffer. Any previous "
	 "contents of the " SHELLBUFF " buffer are deleted."
	},
	{"copy-region",			ZCOPY_REGION,		AI,
	 "Copies the region to the kill buffer. The kill buffer is "
	 "overwritten unless the the delete flag is set. See Append Kill "
	 "command."
	},
	{"copy-word",			ZCOPY_WORD,		AI,
	 "Copies the word the Point is on to the kill buffer. The kill buffer "
	 "is overwritten unless the the delete flag is set.In the PAW, the "
	 "Copy Word command takes the word the Point was on in the "
	 "previously active window and inserts it into the PAW."
	},
	{"count",			ZCOUNT,			0,
	 "Count the number of lines, words, and characters in the region. A "
	 "Universal Argument counts the entire buffer."
	},
	{"ctrl-x",			ZCTRL_X,		0,
	 "Command prefix."
	},
	{"delete-blanks",		ZDELETE_BLANKS,		AN,
	 "Delete all the blank lines around the Point. The lines are not put "
	 "in the Kill Buffer."
	},
	{"delete-buffer",		ZDELETE_BUFFER,		0,
	 "Deletes the current buffer and goes to a previous buffer. There "
	 "must always be at least one buffer. If the buffer has been "
	 "modified, Zedit asks to save it before deleting it. A Universal "
	 "Argument prompts for the buffer to delete."
	},
	{"delete-char",			ZDELETE_CHAR,		AN,
	 "Deletes the character at the Point and leaves the Point on the "
	 "next character in the buffer. The character is not put in the Kill "
	 "Buffer."
	},
	{"delete-line",			ZDELETE_LINE,		AN,
	 "Deletes the entire line, including the Newline, no matter where "
	 "the Point is in the line. The Point is left at the start of the "
	 "next line. The deleted line is put in the Kill Buffer."
	},
	{"delete-previous-char",	ZDELETE_PREVIOUS_CHAR,	AN,
	 "Deletes the character before the Point and leaves the Point in the "
	 "same place. The character is not put in the Kill Buffer."
	},
	{"delete-previous-word",	ZDELETE_PREVIOUS_WORD,	AN,
	 "Deletes the word to the left of the Point. The character the Point "
	 "is on is not deleted. The word is put in the Kill Buffer."
	},
	{"delete-region",		ZDELETE_REGION,		AI,
	 "Deletes the characters in the region. The deleted characters are "
	 "put in the Kill Buffer based on the delete flag."
	},
	{"delete-to-eol",		ZDELETE_TO_EOL,		AN,
	 "Deletes the characters from the Point to the end of the line. If "
	 "the Point is at the end of a line, the Newline character is "
	 "deleted and the next line is joined to the end of the current "
	 "line. The characters deleted are put in the Kill Buffer."
	},
	{"delete-word",			ZDELETE_WORD,		AN,
	 "Deletes the word to the right of and including the Point. The word "
	 "is put in the Kill Buffer."
	},
	{"empty-buffer",		ZEMPTY_BUFFER,		AI,
	 "Deletes the entire contents of the current buffer."
	},
	{"end-of-buffer",		ZEND_OF_BUFFER,		AI,
	 "Moves the Point to the end of the buffer."
	},
	{"end-of-line",			ZEND_OF_LINE,		AN,
	 "Moves the Point to the end of the current line or to the end of the "
	 "next line if already at the end of the current line."
	},
	{"exit",			ZEXIT,			0,
	 "Exits from the editor. It asks to save all modified buffers. A "
	 "bang (!) saves all remaining buffers. A Universal Argument causes "
	 "all modified buffers to be saved without prompting."
	},
	{"fill-check",			ZFILL_CHECK,		0,
	 "Checks if the current column is past the FillWidth column. If it "
	 "is, the words past or on the FillWidth column are wrapped. This "
	 "gives some word processing capability to the editor. Normally bound "
	 "to the space bar and Newline only in Text Mode."
	},
	{"fill-paragraph",		ZFILL_PARAGRAPH,	0,
	 "Uses the FillWidth to reformat the paragraph the Point is in. This "
	 "is useful if editing has messed up the right margin. A Universal "
	 "Argument reformats the next Arg paragraphs. A Universal Argument "
	 "of 0 reformats the entire buffer. When reformatting the entire "
	 "buffer, hitting a character will abort the reformat. Not allowed "
	 "in program mode buffers."
	},
	{"find-file",			ZFIND_FILE,		AN,
	 "Prompts for a path name. If a buffer already exists with this path "
	 "name, that buffer is switched to. If no buffer is matched, a new "
	 "buffer is created and the file read into it. Supports file name "
	 "completion."
	},
	{"global-re-search",		ZGLOBAL_RE_SEARCH,	AN,
	 "This command is used to search for a regular expression in all the "
	 "buffers. It prompts for a search string and then starts searching "
	 "at the start of the first buffer. If a match is found, it stops "
	 "and leaves the point at the match. If Again is executed, it starts "
	 "at the current Point and searches forwards through the buffers. At "
	 "the last buffer it stops and puts the Point back where it "
	 "started."
	},
	{"global-search",		ZGLOBAL_SEARCH,		AN,
	 "This command is used to search for a string in all the buffers. It "
	 "prompts for a search string and then starts searching at the start "
	 "of the first buffer. If a match is found, it stops and leaves the "
	 "point at the match. If Again is executed, it starts at the current "
	 "Point and searches forwards through the buffers. At the last "
	 "buffer it stops and puts the Point back where it started."
	},
	{"goto-line",			ZGOTO_LINE,		0,
	 "Moves the point to the start of a given line. If there is a "
	 "Universal Argument, uses the argument, else prompts for the line "
	 "number. If the line is past the end of the buffer, moves the Point "
	 "is left at the end of the buffer."
	},
	{"grep",			ZGREP,			0,
	 "Prompts for the completion of a grep command and then performs the "
	 "grep command in the " MAKEBUFF " buffer. A Universal Argument will "
	 "prompt for the grep command to use. Use next-error to go through the "
	 "grep output."
	},
	{"grow-window",			ZGROW_WINDOW,		AN,
	 "Makes the active window one line bigger, and therefore the "
	 "inactive window one line smaller. A window cannot be less than "
	 "three (3) lines. If there is only one window, this command has no "
	 "effect."
	},
	{"help",			ZHELP,			AI,
	 "Interface to help functions. Mimics the emacs C-H command prefix."
	},
	{"help-apropos",		ZHELP_APROPOS,		AI,
	 "Asks for a string and then searches through all the functions and "
	 "returns the functions that have the string in them."
	},
	{"help-function",		ZHELP_FUNCTION,		AI,
	 "Displays help on any of the Zedit functions. Prompts for the "
	 "function with full completion. After the name is an optional set of "
	 "flags. Currently they are (+-P). A + means a Universal Arg causes "
	 "the command to repeat. A - means a Universal Arg is ignored. A P "
	 "means the command works in the PAW. If the command has any key "
	 "bindings, these are also displayed."
	},
	{"help-key",			ZHELP_KEY,		AI,
	 "Prompts for a key and displays the current command bound to the key "
	 "in the PAW. Handles C-X and M- prefixes."
	},
	{"help-variable",		ZHELP_VARIABLE,		AI,
	 "Displays help on any of the configurable variables. Prompts for "
	 "the variable with full completion. The current value of the variable "
	 "is also displayed."
	},
	{"incremental-search",		ZINCREMENTAL_SEARCH,	0,
	 "Searches for the string after every character is entered in the "
	 "PAW. The delete-previous-character command can be used to delete a "
	 "character and 'back up' the search. The Newline character "
	 "terminates the search. The Abort command terminates the search and "
	 "places the Point back where it was. Any other commands terminates "
	 "the search and performs that command. A Universal Argument causes "
	 "the search to search backwards."
	},
	{"indent",			ZINDENT,		AN,
	 "Indents the marked region."
	},
	{"insert",			ZINSERT,		AN,
	 "Normally bound to all the printable characters."
	},
	{"insert-overwrite",		ZINSERT_OVERWRITE,	AI,
	 "Toggles the current buffer's minor mode between insert, the default, "
	 "and overwrite modes."
	},
	{"join",			ZJOIN,			AN,
	 "Joins two lines. Performs the following Zedit commands: End of "
	 "Line, Delete Newline, Trim Whitespace, Insert space."
	},
	{"kill",			ZKILL,			AI,
#if !DOPIPES
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Kills the current make. The command must wait for the make to die "
	 "before it can continue. Unix only."
	},
	{"list-buffers",		ZLIST_BUFFERS,		AI,
	 "Displays a list of the current buffers on the display."
	},
	{"lowercase-region",		ZLOWERCASE_REGION,	AI,
	 "Converts the Region to lowercase. Not allowed in program mode "
	 "buffers."
	},
	{"lowercase-word",		ZLOWERCASE_WORD,	AN,
	 "Converts the current word starting at the Point to lowercase. It "
	 "leaves the Point at the end of the word."
	},
	{"make",			ZMAKE,			0,
	 "This command performs the command specified in the Make variable "
	 "and puts the output in the " MAKEBUFF " buffer. The command is "
	 "usually 'make' and is used in conjunction with the next-error "
	 "command. A Universal Argument prompts for the command to execute."
	},
	{"mark-paragraph",		ZMARK_PARAGRAPH,	AN,
	 "Sets the Mark to the start of the current paragraph and moves the "
	 "Point to the start of the next paragraph."
	},
	{"meta",			ZMETA,			0,
	 "Command prefix."
	},
	{"meta-x",			ZMETA_X,		0,
	 "Prompts for a command to execute. Unbound commands may be executed "
	 "in this manner. Supports command completion. A '?' will show all "
	 "the possible matches. A Universal Argument is passed on to the "
	 "selected command."
	},
	{"mode",			ZMODE,			AN,
	 "Change the mode of the current buffer. Prompts (with command "
	 "completion) for the mode to change to."
	},
	{"newline",			ZNEWLINE,		AN,
	 "The command normally bound to the Enter or Return key. In "
	 "overwrite mode, a NL goes to start of next line. In insert mode, "
	 "its just inserted."
	},
	{"next-bookmark",		ZNEXT_BOOKMARK,		0,
	 "Moves the Point to the last bookmark set in the bookmark ring. The "
	 "bookmark moved to is displayed in the echo window. A Universal "
	 "Argument in the range 1 to 10 corresponding to a set bookmark will "
	 "go to the bookmark."
	},
	{"next-buffer",			ZNEXT_BUFFER,		AN,
	 "Switches to the next buffer in the buffer list. At the end of the "
	 "list, it switches to the first buffer in the list, i.e. treats the "
	 "list like a ring."
	},
	{"next-char",			ZNEXT_CHAR,		AN,
	 "Moves the Point forward one character. If the Point is at the end "
	 "of a line, it is moved to the start of the next line."
	},
	{"next-error",			ZNEXT_ERROR,		AN,
	 "Used after a make or grep command to search for error messages, "
	 "or grep output, "
	 "in the " MAKEBUFF " buffer. If an error is found, the "
	 "Mark in the " MAKEBUFF " buffer is placed at the start of the "
	 "error message. The file containing the error is loaded into a "
	 "buffer using the Find File command and the Point is placed at "
	 "the error line."
	},
	{"next-line",			ZNEXT_LINE,		AN,
	 "Moves the Point up one line in the buffer. It tries to maintain the "
	 "same column position. If the line is to short the, Point will be "
	 "the placed at the end of the line. Consecutive Previous/Next Line "
	 "or Page commands try to maintain the original column position."
	},
	{"next-page",			ZNEXT_PAGE,		AN,
	 "Moves the Point down one page and tries to center the Point line in "
	 "the display. It tries to maintain the same column position. If the "
	 "line is to short the, Point will be the placed at the end of the "
	 "line. Consecutive Previous/Next Line or Page commands try to "
	 "maintain the original column position."
	},
	{"next-paragraph",		ZNEXT_PARAGRAPH,	AN,
	 "Moves the Point to the start of the next paragraph."
	},
	{"next-window",			ZNEXT_WINDOW,		AN,
	 "Goes to the next window on the screen. The windows are treated as "
	 "a circular list."
	},
	{"next-word",			ZNEXT_WORD,		AN,
	 "Moves the Point to the start of the next word."
	},
	{"notimpl",			ZNOTIMPL,		AI,
	 "Used to unbind a key. This is bound to all the unbound keys."
	},
	{"one-window",			ZONE_WINDOW,		AI,
	 "Makes the active window a full screen window. If there is only one "
	 "window, this command has no effect."
	},
	{"open-line",			ZOPEN_LINE,		AN,
	 "Inserts a Newline at the Point but leaves the Point in front of "
	 "the Newline. It is the same as typing a Newline and then a "
	 "Previous Character command."
	},
	{"other-next-page",		ZOTHER_NEXT_PAGE,	AN,
	 "Performs a Next Page command in the bottom window or the top window "
	 "if you are in the bottom window. It leaves the Point where it is "
	 "in the active window. If there is only one window, this command "
	 "is the same as next-page."
	},
	{"other-previous-page",		ZOTHER_PREVIOUS_PAGE,	AN,
	 "Performs a Previous Page command in the bottom window or the top "
	 "window if you are in the bottom window. It leaves the Point where "
	 "it is in the active window. If there is only one window, it performs "
	 "a previous-page."
	},
	{"out-to",			ZOUT_TO,		0,
	 "This command moves the Point to an absolute column position. If the "
	 "line is shorter than the specified column, it is padded with tabs "
	 "and spaces to the specified column. The command takes either a "
	 "Universal Argument or prompts for the column to go to."
	},
	{"position",			ZPOSITION,		AI,
	 "Displays the current Point position as a line, column, and byte "
	 "offset in the echo window. Also displays the length of the "
	 "buffer."
	},
	{"previous-char",		ZPREVIOUS_CHAR,		AN,
	 "Moves the Point back one character. If the Point is at the start of "
	 "the line, it is moved to the end of the previous line."
	},
	{"previous-line",		ZPREVIOUS_LINE,		AN,
	 "Moves the Point up one line in the buffer. It tries to maintain the "
	 "same column position. If the line is to short, the Point will be "
	 "placed at the end of the line. Consecutive Previous/Next Line or "
	 "Page commands try to maintain the original column position."
	},
	{"previous-page",		ZPREVIOUS_PAGE,		AN,
	 "Moves the Point up one page and tries to center the Point line in "
	 "the display. It tries to maintain the same column position. If the "
	 "line is to short, the Point will be placed at the end of the "
	 "line. Consecutive Previous/Next Line or Page commands try to "
	 "maintain the original column position."
	},
	{"previous-paragraph",		ZPREVIOUS_PARAGRAPH,	AN,
	 "Moves the Point to the start of the paragraph or to the start of "
	 "the previous paragraph."
	},
	{"previous-word",		ZPREVIOUS_WORD,		AN,
	 "Moves the Point back to the start of a word or to the start of the "
	 "previous word."
	},
	{"query-replace",		ZQUERY_REPLACE,		0,
	 "Prompts for a search string and a replacement string and searches "
	 "from the current Point looking for matches of the search string. "
	 "If it finds a match, it moves the Point to the match and waits for "
	 "one of the following input characters:\n\n"
	 "  ,              Replace current match and confirm.\n"
	 "  space          Replace current match and continue.\n"
	 "  y              Replace current match and continue.\n"
	 "  .              Replace and exit.\n"
	 "  !              Global replace.\n"
	 "  ^              Goto previous match.\n"
	 "  Abort          Exit without replacing current match.\n"
	 "  Others         Continue without replacing current match.\n"
	 "\n"
	 "The Point is left at the position it was in before the Query "
	 "Replace. If the mode Exact is set, the search will be case "
	 "sensitive. A Universal Argument causes the replacement to be "
	 "done globally in all the buffers."
	},
	{"quote",			ZQUOTE,			AN,
	 "The Quote command is used to insert a character into a buffer or "
	 "String Argument that would normally be a command. The next "
	 "character after the Quote command is taken literally and inserted "
	 " into the buffer."
	},
	{"re-replace",			ZRE_REPLACE,		0,
	 "Works like the Query Replace command except that the search string "
	 "is a regular expression. The replacement string is a literal with "
	 "two exceptions. An '&' character causes the matched string to be "
	 "placed in the buffer. The escape character '\' can be used to turn "
	 "off this special meaning of '&'. Note that '\\' is required to put "
	 "a real '\' in the buffer. For each match, a prompt is made for the "
	 "action to perform. See Query Replace for a list of valid "
	 "actions. A Universal Argument causes the replacement to be done "
	 "globally in all the buffers."
	},
	{"re-search",			ZRE_SEARCH,		AN,
	 "Asks for a regular expression search string and searches from the "
	 "Point forward for a match in the buffer. If a match is found, the "
	 "Point is moved to the start of the match. If the string is not "
	 "found, then 'Not Found' is displayed in the echo window and the "
	 "Point is left where it was. The search string is saved and "
	 "subsequent search and replace commands will default to this "
	 "string. If the mode Exact is set, the search will be case "
	 "sensitive."
	},
	{"read-file",			ZREAD_FILE,		0,
	 "Prompts for a path name, with file name completion, and inserts "
	 "the file into the current buffer before the Point. If there is a "
	 "Universal Argument, the current buffer is deleted, first asking to "
	 "save the file if it is modified, before reading in the new "
	 "file. The buffers file name is not changed, any writes will be to "
	 "the old file name."
	},
	{"redisplay",			ZREDISPLAY,		AN,
	 "Repaints the entire screen."
	},
	{"replace",			ZREPLACE,		0,
	 "Prompts for a search string and a replacement string and searches "
	 "from the current Point looking for matches of the search "
	 "string. If it finds a match, it replaces the string and "
	 "continues. The Point is left at the position it was in before the "
	 "Query Replace. If the mode Exact is set, the search will be case "
	 "sensitive. A Universal Argument causes the replacement to be done "
	 "globally in all the buffers."
	},
	{"reverse-search",		ZREVERSE_SEARCH,	AN,
	 "Prompts for a search string and searches from the Point backward "
	 "for a match in the buffer. If a match is found, the Point is moved "
	 "to the start of the match. If the string is not found, then 'Not "
	 "Found' is displayed in the echo window and the Point is not "
	 "moved. The search string is saved and subsequent search and "
	 "replace commands will default to the last search string. If the "
	 "mode Exact is set, the search will be case sensitive."
	},
	{"revert-file",			ZREVERT_FILE,		AN,
	 "Rereads the current file. If the file has changed, asks to "
	 "overwrite changes. A Universal Argument causes multiple "
	 "rereads. Mimics the vi 'e' command."
	},
	{"save-all-files",		ZSAVE_ALL_FILES,	0,
	 "Saves all modified buffers. A Universal Argument causes ALL files "
	 "to be saved, modified or not."
	},
	{"save-and-exit",		ZSAVE_AND_EXIT,		0,
	 "An attempt to mimic the vi ZZ command. Does a save file on the "
	 "current buffer and then exits. A universal argument will save all "
	 "files."
	},
	{"save-file",			ZSAVE_FILE,		AI,
	 "Saves the current buffer to its file. If the current buffer has no "
	 "path name, a path name is prompted for (with file name "
	 "completion)."
	},
	{"scroll-down",			ZSCROLL_DOWN,		AN,
	 "Scrolls the screen down one line."
	},
	{"scroll-up",			ZSCROLL_UP,		AN,
	 "Scrolls the screen up one line."
	},
	{"search",			ZSEARCH,		AN,
	 "Prompts for a search string and searches from the Point forward for "
	 "a match in the buffer. If a match is found, the Point is moved to "
	 "the start of the match. If the string is not found, then 'Not "
	 "Found' is displayed in the echo window and the Point is not "
	 "moved. The search string is saved and subsequent search or replace "
	 "commands will default to the last search string. If the mode Exact "
	 "is set, the search will be case sensitive."
	},
	{"set-bookmark",		ZSET_BOOKMARK,		AN,
	 "Places an invisible 'bookmark' at the Point. There are 10 "
	 "bookmarks placed in a ring. The bookmark set is displayed in the "
	 "PAW."
	},
	{"set-mark",			ZSET_MARK,		AI,
	 "Sets the Mark at the Point."
	},
	{"set-variable",		ZSET_VARIABLE,		0,
	 "Allows any of the configurable variables to be set. Command "
	 "completion is supported. Prompts for the variable, then prompts "
	 "for the new setting. A Universal Argument causes numeric or flag "
	 "variables to be set to Arg, string variables ignore the Universal "
	 "Argument."
	},
	{"setenv",			ZSETENV,		AN,
	 "Performs a 'setenv' command on an environment variable. The "
	 "variable will keep the setting during the the Zedit session."
	},
	{"show-config",			ZSHOW_CONFIG,		AI,
	 "Show the current settings of all the Zedit variables in a "
	 "buffer. The buffer is suitable for use as a .config.z file and can "
	 "be saved to a file using Write File."
	},
	{"size-window",			ZSIZE_WINDOW,		0,
	 "Sets the window to Universal Argument lines."
	},
	{"spell-word",			ZSPELL_WORD,		0,
#if !SPELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Check the spelling of the word at the point. Give hints about "
	 "possible correct spellings."
	},
	{"split-window",		ZSPLIT_WINDOW,		AN,
	 "Splits the current window into two windows. The same buffer is "
	 "displayed in both windows and the bottom window is made active."
	},
	{"stats",			ZSTATS,			AI,
	 "Displays some simple stats about Zedit."
	},
	{"swap-chars",			ZSWAP_CHARS,		AN,
	 "Swaps the character before the Point with the character at the "
	 "Point. It leaves the Point after the second character. Successive "
	 "commands will 'drag' the character that was before the Point "
	 "towards the end of the line. If the Point was at the end of a line, "
	 "the two characters before the Point are transposed. If the Point is "
	 "at the beginning of the buffer, the character at the Point and the "
	 "character after the Point are transposed."
	},
	{"swap-mark",			ZSWAP_MARK,		AI,
	 "The Mark goes to the current position of the Point and the Point is "
	 "set where the Mark was."
	},
	{"swap-words",			ZSWAP_WORDS,		AN,
	 "Swaps the words before and after the Point. The Point is left "
	 "after the second word. Successive commands will 'drag' the word "
	 "before the Point towards the end of the line. If the Point is at "
	 "the end of a line, the last word on the line is transposed with "
	 "the first word on the next line. The command does nothing at the "
	 "end of the buffer. If the Point is in the middle of a word, the "
	 "two halves of the word are transposed."
	},
	{"switch-to-buffer",		ZSWITCH_TO_BUFFER,	AI,
	 "Prompts for a buffer name to switch to with command completion. "
	 "The previous buffer is stored and displayed as a default. This "
	 "allows quick switching between two buffers. The Again command can "
	 "be used to scroll through the buffer list."
	},
	{"tab",				ZTAB,			AN,
	 "Handles the tab key."
	},
	{"tag",				ZTAG,			0,
	 "The tag command asks for a function and looks for the function in a "
	 "tagfile. If there is no tagfile specified, then it will prompt for a "
	 "tagfile. A Universal Argument always prompts for a tagfile.\n\n"
	 "Only supports etags style tagfiles."
	},
	{"tag-word",			ZTAG_WORD,		0,
	 "Like the tag command but uses the word at the point for the search tag."
	},
	{"toggle-case",			ZTOGGLE_CASE,		AI,
	 "Switches the current buffers minor mode between case sensitive and "
	 "case insensitive for searches and replacements."
	},
	{"trim-white-space",		ZTRIM_WHITE_SPACE,	AI,
	 "Removes all spaces and tabs on both sides of the Point. The Point "
	 "is left on the character that is to the right of the deleted "
	 "text. The deleted characters are not put in the Kill Buffer."
	},
	{"undent",			ZUNDENT,		AN,
	 "Removes Universal Argument tabs from the start of every line in "
	 "the region."
	},
	{"undo",			ZUNDO,			AN,
#if !UNDO
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "EXPERIMENTAL: Undo the previous edit. There is a list of undos."
	},
	{"unmodify",			ZUNMODIFY,		AI,
	 "Turns off the modified flag for the current buffer. Does not change "
	 "the buffer itself."
	},
	{"uppercase-region",		ZUPPERCASE_REGION,	AI,
	 "Convert the Region to uppercase. Not allowed in program mode "
	 "buffers."
	},
	{"uppercase-word",		ZUPPERCASE_WORD,	AN,
	 "Converts the current word starting at the Point to uppercase. It "
	 "leaves the Point at the end of the word."
	},
	{"view-line",			ZVIEW_LINE,		AN,
	 "Makes the Point line the top line in the window."
	},
	{"word-search",			ZWORD_SEARCH,		AN,
	 "Search for the word at the point."
	},
	{"write-file",			ZWRITE_FILE,		0,
	 "Prompts for a path name and writes out the current buffer to this "
	 "file name. Supports file name completion. It changes the path name "
	 "of the current buffer to the new path name. A Universal Argument "
	 "causes only the Region to be written and does not change the name "
	 "of the buffer."
	},
	{"yank",			ZYANK,			AN,
	 "Inserts the characters from the Kill Buffer before the Point. The "
	 "characters are inserted, even in overwrite mode."
	},
};
