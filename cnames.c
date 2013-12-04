#include "z.h"

struct cnames Cnames[] = {
	{"abort",			ZABORT,
	 "Aborts the current command. This is the only way to exit Universal "
	 "and String Arguments. It will also abort C-X and Meta prefixes."
	},
	{"again",			ZAGAIN,
	 "Causes the last search to be executed again. If the mode Exact is "
	 "set, the search will be case sensitive. A Universal Argument "
	 "causes the search to look for an exact number matches of the "
	 "string. If the number cannot be matched, it will display the "
	 "number actually matched in the PAW and leave the Point where it "
	 "was. If no search commands have been executed, a Forward Search is "
	 "executed."
	},
	{"append-kill",			ZAPPEND_KILL,
	 "Sets the delete flag. The next delete command will append to the "
	 "kill buffer. A Universal Argument is ignored."
	},
	{"arg",				ZARG,
	 "Many of the commands accept a Universal Argument to cause the "
	 "command to repeat or to modify the meaning of the command in some "
	 "way. It reads decimal digits and displays them in the PAW. The "
	 "first non-digit is processed as a command. The digits cannot be "
	 "deleted since Delete is a valid command that accepts a Universal "
	 "Argument. If a mistake is made, use the Abort Command."
	},
	{"beginning-of-buffer",		ZBEGINNING_OF_BUFFER,
	 "Moves the Point to the beginning of the current buffer. A "
	 "Universal Argument is ignored."
	},
	{"beginning-of-line",		ZBEGINNING_OF_LINE,
	 "Moves the Point to the beginning of the line or to the beginning of "
	 "the previous line. A Universal Argument causes the command to "
	 "repeat."
	},
	{"bind",			ZBIND,
	 "Binds a function to a key. It first prompts, with command "
	 "completion, for the command to bind. It then prompts for the key "
	 "to bind the command to. The Meta and C-X prefix keys are entered, "
	 "Bind prompts for another key. The Abort command exits any "
	 "prompt. The Quote command can be used to rebind the Abort, Quote, "
	 "C-X, and Meta commands. A Universal Argument causes the Bind "
	 "command to reset all the default arguments. Keys can be 'unbound' "
	 "by binding them to the NULL command."
	},
	{"c-indent",			ZC_INDENT,
	 "Causes Newline characters to auto-indent to the current tab "
	 "level. If a Newline is hit after an open brace ({), the next line "
	 "is indented an extra tab stop. Bound to Newline in C mode. A "
	 "Universal Argument causes the command to repeat."
	},
	{"c-insert",			ZC_INSERT,
	 "Tries to perform brace matching. When a close brace (}) is "
	 "entered, searches for a previous unmatched open brace ({). If it "
	 "finds one, tries to put the close brace in the same column as the "
	 "column of the first non-whitespace character in the matched "
	 "line. If no match is found, beeps. Bound to close brace in C "
	 "mode. If bound to a character other than a close brace, performs a "
	 "Character Insert. A Universal Argument is ignored."
	},
	{"calc",			ZCALC,
	 "This is a very simple integer calculator. It handles addition, "
	 "subtraction, multiplication, and division between "
	 "octal/decimal/hexadecimal numbers. If a number begins with a '0x' "
	 "it is assumed to be hexadecimal, if it begins with a '0' it is "
	 "assumed to be octal, all others are assumed to be decimal. The "
	 "output is displayed in decimal with hexadecimal in brackets "
	 "'()'. Spaces and tabs are allowed but are not necessary between "
	 "the numbers and their operators. A Universal Argument is "
	 "ignored."
	},
	{"capitalize-word",		ZCAPITALIZE_WORD,
	 "In C mode, converts the character at the Point to uppercase and the "
	 "rest of the word to lowercase. In other modes, converts the first "
	 "letter of the current word to uppercase and the rest of the "
	 "characters in the word to lowercase. The Point is left at the end "
	 "of the word. A Universal Argument causes the command to repeat."
	},
	{"center",			ZCENTER,
	 "Centers the current line on the screen. It uses fill-width to define "
	 "the right margin. A Universal Argument causes the command to "
	 "repeat on the next lines."
	},
	{"cmd-to-buffer",		ZCMD_TO_BUFFER,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Prompts for a command and then executes the command in the "
	 "shell. The output is put in the '.shell' buffer. Any previous "
	 "contents of the '.shell' buffer are deleted. A Universal Argument "
	 "is ignored."
	},
	{"copy-region",			ZCOPY_REGION,
	 "Copies the region to the kill buffer. The kill buffer is "
	 "overwritten unless the the delete flag is set. See Append Kill "
	 "command."
	},
	{"copy-word",			ZCOPY_WORD,
	 "Copies the word the Point is on to the kill buffer. The kill buffer "
	 "is overwritten unless the the delete flag is set.In the PAW, the "
	 "Copy Word command takes the word the Point was on in the "
	 "previously active window and inserts it into the PAW."
	},
	{"count",			ZCOUNT,
	 "Count the number of lines, words, and characters in the region. A "
	 "Universal Argument counts the entire buffer."
	},
	{"ctrl-x",			ZCTRL_X,
	 "Command prefix."
	},
	{"delete-blanks",		ZDELETE_BLANKS,
	 "Delete all the blank lines around the Point. The lines are not put "
	 "in the Kill Buffer. A Universal Argument causes the command to "
	 "repeat, which accomplishes nothing."
	},
	{"delete-buffer",		ZDELETE_BUFFER,
	 "Deletes the current buffer and goes to a previous buffer. There "
	 "must always be at least one buffer. If the buffer has been "
	 "modified, Zedit asks to save it before deleting it. A Universal "
	 "Argument prompts for the buffer to delete."
	},
	{"delete-char",			ZDELETE_CHAR,
	 "Deletes the character at the Point and leaves the Point on the "
	 "next character in the buffer. The character is not put in the Kill "
	 "Buffer. A Universal Argument causes the command to repeat."
	},
	{"delete-line",			ZDELETE_LINE,
	 "Deletes the entire line, including the Newline, no matter where "
	 "the Point is in the line. The Point is left at the start of the "
	 "next line. The deleted line is put in the Kill Buffer. A Universal "
	 "Argument causes the command to repeat."
	},
	{"delete-previous-char",	ZDELETE_PREVIOUS_CHAR,
	 "Deletes the character before the Point and leaves the Point in the "
	 "same place. The character is not put in the Kill Buffer. A "
	 "Universal Argument causes the command to repeat."
	},
	{"delete-previous-word",	ZDELETE_PREVIOUS_WORD,
	 "Deletes the word to the left of the Point. The character the Point "
	 "is on is not deleted. The word is put in the Kill Buffer. A "
	 "Universal Argument causes the command to repeat."
	},
	{"delete-region",		ZDELETE_REGION,
	 "Deletes the characters in the region. The deleted characters are "
	 "put in the Kill Buffer based on the delete flag. A Universal "
	 "Argument is ignored."
	},
	{"delete-to-eol",		ZDELETE_TO_EOL,
	 "Deletes the characters from the Point to the end of the line. If "
	 "the Point is at the end of a line, the Newline character is "
	 "deleted and the next line is joined to the end of the current "
	 "line. The characters deleted are put in the Kill Buffer. A "
	 "Universal Argument causes the command to repeat."
	},
	{"delete-word",			ZDELETE_WORD,
	 "Deletes the word to the right of and including the Point. The word "
	 "is put in the Kill Buffer. A Universal Argument causes the command "
	 "to repeat."
	},
	{"empty-buffer",		ZEMPTY_BUFFER,
	 "Deletes the entire contents of the current buffer. A Universal "
	 "Arguments is ignored."
	},
	{"end-of-buffer",		ZEND_OF_BUFFER,
	 "Moves the Point to the end of the buffer. A Universal Argument is "
	 "ignored."
	},
	{"end-of-line",			ZEND_OF_LINE,
	 "Moves the Point to the end of the line or to the end of the next "
	 "line. A Universal Argument causes the command to repeat."
	},
	{"exit",			ZEXIT,
	 "Exits from the editor. It asks to save all modified buffers. A "
	 "bang (!) saves all remaining buffers. A Universal Argument causes "
	 "all modified buffers to be saved without prompting."
	},
	{"fill-check",			ZFILL_CHECK,
	 "Checks if the current column is past the FillWidth column. If it "
	 "is, the words past or on the FillWidth column are wrapped. This "
	 "gives some word processing capability to the editor. Normally bound "
	 "to the space bar and Newline only in Text Mode. A Universal "
	 "Argument has no meaning."
	},
	{"fill-paragraph",		ZFILL_PARAGRAPH,
	 "Uses the FillWidth to reformat the paragraph the Point is in. This "
	 "is useful if editing has messed up the right margin. A Universal "
	 "Argument reformats the next Arg paragraphs. A Universal Argument "
	 "of 0 reformats the entire buffer. When reformatting the entire "
	 "buffer, hitting a character will abort the reformat. Not allowed "
	 "in program mode buffers."
	},
	{"find-file",			ZFIND_FILE,
	 "Prompts for a path name. If a buffer already exists with this path "
	 "name, that buffer is switched to. If no buffer is matched, a new "
	 "buffer is created and the file read into it. Supports file name "
	 "completion. A Universal Argument causes the command to repeat."
	},
	{"global-re-search",		ZGLOBAL_RE_SEARCH,
	 "This command is used to search for a regular expression in all the "
	 "buffers. It prompts for a search string and then starts searching "
	 "at the start of the first buffer. If a match is found, it stops "
	 "and leaves the point at the match. If Again is executed, it starts "
	 "at the current Point and searches forwards through the buffers. At "
	 "the last buffer it stops and puts the Point back where it "
	 "started. A Universal Argument is ignored."
	},
	{"global-search",		ZGLOBAL_SEARCH,
	 "This command is used to search for a string in all the buffers. It "
	 "prompts for a search string and then starts searching at the start "
	 "of the first buffer. If a match is found, it stops and leaves the "
	 "point at the match. If Again is executed, it starts at the current "
	 "Point and searches forwards through the buffers. At the last "
	 "buffer it stops and puts the Point back where it started. A "
	 "Universal Argument is ignored."
	},
	{"goto-line",			ZGOTO_LINE,
	 "Moves the point to the start of a given line. If there is a "
	 "Universal Argument, uses the argument, else prompts for the line "
	 "number. If the line is past the end of the buffer, moves the Point "
	 "is left at the end of the buffer."
	},
	{"grep",			ZGREP,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Prompts for the completion of a grep command and then performs the "
	 "grep command in the *grep* buffer."
	},
	{"grow-window",			ZGROW_WINDOW,
	 "Makes the active window one line bigger, and therefore the "
	 "inactive window one line smaller. A window cannot be less than "
	 "three (3) lines. If there is only one window, this command has no "
	 "effect. A Universal Argument causes the command to grow the window "
	 "by that number of lines."
	},
	{"help",			ZHELP,
	 "Interface to help functions. Mimics the emacs C-H command prefix."
	},
	{"help-apropos",		ZHELP_APROPOS,
	 "Asks for a string and then searches through all the functions and "
	 "returns the functions that have the string in them."
	},
	{"help-function",		ZHELP_FUNCTION,
	 "Displays help on any of the Zedit functions. Prompts for the "
	 "function with full completion. If the command has any key bindings, "
	 " these are also displayed."
	},
	{"help-key",			ZHELP_KEY,
	 "Prompts for a key and displays the current command bound to the key "
	 "in the PAW. Handles C-X and M- prefixes. A Universal Argument is "
	 "ignored."
	},
	{"help-variable",		ZHELP_VARIABLE,
	 "Displays help on any of the configurable variables. Prompts for "
	 "the variable with full completion. The current value of the variable "
	 "is also displayed."
	},
	{"incremental-search",		ZINCREMENTAL_SEARCH,
	 "Searches for the string after every character is entered in the "
	 "PAW. The Delete Previous Character command can be used to delete a "
	 "character and 'back up' the search. The Newline character "
	 "terminates the search. The Abort command terminates the search and "
	 "places the Point back where it was. Any other commands terminates "
	 "the search and performs that command. A Universal Argument causes "
	 "the command to repeat."
	},
	{"indent",			ZINDENT,
	 "Indents the marked region Universal Argument tab stops."
	},
	{"insert",			ZINSERT,
	 "Normally bound to the printable characters, causes the character to "
	 "be inserted in the buffer. A Universal Argument causes the command "
	 "to repeat."
	},
	{"insert-overwrite",		ZINSERT_OVERWRITE,
	 "Toggles the current buffers minor mode between Insert, the default, "
	 "and overwrite modes. On the PC the cursor shape reflects the "
	 "mode. Insert mode has an underline cursor while overwrite mode has "
	 "a block cursor. A Universal Argument is ignored."
	},
	{"join",			ZJOIN,
	 "Joins two lines. Performs the following Zedit commands: End of "
	 "Line, Delete Newline, Trim Whitespace, Insert space."
	},
	{"kill",			ZKILL,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Kills the current make. The command must wait for the make to die "
	 "before it can continue. Unix only."
	},
	{"life",			ZLIFE,
	 "Destructively plays the game of Life on the current buffer. A "
	 "space does the next iteration. A Return causes continuous "
	 "iterations (with a slight delay). An Abort stops the game."
	},
	{"list-buffers",		ZLIST_BUFFERS,
	 "Displays a list of the current buffers on the display."
	},
	{"lowercase-region",		ZLOWERCASE_REGION,
	 "Converts the Region to lowercase. Not allowed in program mode "
	 "buffers. A Universal Argument is ignored."
	},
	{"lowercase-word",		ZLOWERCASE_WORD,
	 "Converts the current word starting at the Point to lowercase. It "
	 "leaves the Point at the end of the word. A Universal Argument "
	 "causes the command to repeat."
	},
	{"make",			ZMAKE,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "This command performs the command specified in the Make variable "
	 "and puts the output in the '.make' buffer. The command is usually "
	 "'make' and is used in conjunction with the Next Error command. A "
	 "Universal Argument prompts for the command to execute."
	},
	{"mark-paragraph",		ZMARK_PARAGRAPH,
	 "Sets the Mark to the start of the current paragraph and moves the "
	 "Point to the start of the next paragraph. A Universal Argument "
	 "causes the command to repeat."
	},
	{"meta",			ZMETA,
	 "Command prefix."
	},
	{"meta-x",			ZMETA_X,
	 "Prompts for a command to execute. Unbound commands may be executed "
	 "in this manner. Supports command completion. A '?' will show all "
	 "the possible matches. A Universal Argument is passed on to the "
	 "selected command."
	},
	{"mode",			ZMODE,
	 "Change the mode of the current buffer. Prompts (with command "
	 "completion) for the mode to change to. A Universal Argument "
	 "causes multiple toggles."
	},
	{"newline",			ZNEWLINE,
	 "The command normally bound to the Enter or Return key. In "
	 "overwrite mode, a NL goes to start of next line. In insert mode, "
	 "its just inserted. A Universal Argument causes the command to "
	 "repeat."
	},
	{"next-bookmark",		ZNEXT_BOOKMARK,
	 "Moves the Point to the last bookmark set in the bookmark ring. The "
	 "bookmark moved to is displayed in the echo window. A Universal "
	 "Argument in the range 1 to 10 corresponding to a set bookmark will "
	 "go to the bookmark."
	},
	{"next-buffer",			ZNEXT_BUFFER,
	 "Switches to the next buffer in the buffer list. At the end of the "
	 "list, it switches to the first buffer in the list, i.e. treats the "
	 "list like a ring. A Universal Argument causes the command to "
	 "repeat."
	},
	{"next-char",			ZNEXT_CHAR,
	 "Moves the Point forward one character. If the Point is at the end "
	 "of a line, it is moved to the start of the next line. A Universal "
	 "Argument causes the command to repeat."
	},
	{"next-error",			ZNEXT_ERROR,
#if !SHELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Used after a Make command to search for error messages in the "
	 "'.make' buffer. If an error is found, the Mark in the '.make' "
	 "buffer is placed at the start of the error message. The file "
	 "containing the error is loaded into a buffer using the Find File "
	 "command and the Point is placed at the error line. If a "
	 "Universal Argument is specified, Next Error tries to ignore "
	 "warnings."
	},
	{"next-line",			ZNEXT_LINE,
	 "Moves the Point up one line in the buffer. It tries to maintain the "
	 "same column position. If the line is to short the, Point will be "
	 "the placed at the end of the line. Consecutive Previous/Next Line "
	 "or Page commands try to maintain the original column position. A "
	 "Universal Argument causes the command to repeat."
	},
	{"next-page",			ZNEXT_PAGE,
	 "Moves the Point down one page and tries to center the Point line in "
	 "the display. It tries to maintain the same column position. If the "
	 "line is to short the, Point will be the placed at the end of the "
	 "line. Consecutive Previous/Next Line or Page commands try to "
	 "maintain the original column position. A Universal Argument causes "
	 "the command to repeat."
	},
	{"next-paragraph",		ZNEXT_PARAGRAPH,
	 "Moves the Point to the start of the next paragraph. A Universal "
	 "Argument causes the command to repeat."
	},
	{"next-window",			ZNEXT_WINDOW,
	 "Goes to the next window on the screen. The windows are treated as "
	 "a circular list. A Universal Argument goes to the Nth next "
	 "window."
	},
	{"next-word",			ZNEXT_WORD,
	 "Moves the Point to the start of the next word. A Universal "
	 "Argument causes the command to repeat."
	},
	{"notimpl",			ZNOTIMPL,
	 "Used to unbind a key. This is bound to all the unbound keys. A "
	 "Universal Argument does nothing Arg times."
	},
	{"one-window",			ZONE_WINDOW,
	 "Makes the active window a full screen window. If there is only one "
	 "window, this command has no effect. A Universal Argument is "
	 "ignored."
	},
	{"open-line",			ZOPEN_LINE,
	 "Inserts a Newline at the Point but leaves the Point in front of "
	 "the Newline. It is the same as typing a Newline and then a "
	 "Previous Character command. A Universal Argument causes the "
	 "command to repeat."
	},
	{"other-next-page",		ZOTHER_NEXT_PAGE,
	 "Performs a Next Page command in the bottom window or the top window "
	 "if you are in the bottom window. It leaves the Point where it is "
	 "in the active window. If there is only one window, this command "
	 "has no effect. A Universal Argument causes the command to page the "
	 "Nth window."
	},
	{"other-previous-page",		ZOTHER_PREVIOUS_PAGE,
	 "Performs a Previous Page command in the bottom window or the top "
	 "window if you are in the bottom window. It leaves the Point where "
	 "it is in the active window. If there is only one window, this "
	 "command has no effect. A Universal Argument pages the Nth "
	 "window."
	},
	{"out-to",			ZOUT_TO,
	 "This command moves the Point to an absolute column position. If the "
	 "line is shorter than the specified column, it is padded with tabs "
	 "and spaces to the specified column. The command takes either a "
	 "Universal Argument or prompts for the column to go to."
	},
	{"position",			ZPOSITION,
	 "Displays the current Point position as a line, column, and byte "
	 "offset in the echo window. Also displays the length of the "
	 "buffer. A Universal Argument is ignored."
	},
	{"previous-char",		ZPREVIOUS_CHAR,
	 "Moves the Point back one character. If the Point is at the start of "
	 "the line, it is moved to the end of the previous line. A Universal "
	 "Argument causes the command to repeat."
	},
	{"previous-line",		ZPREVIOUS_LINE,
	 "Moves the Point up one line in the buffer. It tries to maintain the "
	 "same column position. If the line is to short, the Point will be "
	 "placed at the end of the line. Consecutive Previous/Next Line or "
	 "Page commands try to maintain the original column position. A "
	 "Universal Argument causes the command to repeat."
	},
	{"previous-page",		ZPREVIOUS_PAGE,
	 "Moves the Point up one page and tries to center the Point line in "
	 "the display. It tries to maintain the same column position. If the "
	 "line is to short, the Point will be placed at the end of the "
	 "line. Consecutive Previous/Next Line or Page commands try to "
	 "maintain the original column position. A Universal Argument causes "
	 "the command to repeat."
	},
	{"previous-paragraph",		ZPREVIOUS_PARAGRAPH,
	 "Moves the Point to the start of the paragraph or to the start of "
	 "the previous paragraph. A Universal Argument causes the command to "
	 "repeat."
	},
	{"previous-word",		ZPREVIOUS_WORD,
	 "Moves the Point back to the start of a word or to the start of the "
	 "previous word. A Universal Argument causes the command to repeat."
	},
	{"query-replace",		ZQUERY_REPLACE,
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
	{"quote",			ZQUOTE,
	 "The Quote command is used to insert a character into a buffer or "
	 "String Argument that would normally be a command. The next "
	 "character after the Quote command is taken literally and inserted "
	 " into the buffer. A Universal Argument inserts Arg characters."
	},
	{"re-replace",			ZRE_REPLACE,
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
	{"re-search",			ZRE_SEARCH,
	 "Asks for a regular expression search string and searches from the "
	 "Point forward for a match in the buffer. If a match is found, the "
	 "Point is moved to the start of the match. If the string is not "
	 "found, then 'Not Found' is displayed in the echo window and the "
	 "Point is left where it was. The search string is saved and "
	 "subsequent search and replace commands will default to this "
	 "string. If the mode Exact is set, the search will be case "
	 "sensitive. A Universal Argument causes the search to look for "
	 "exactly Arg matches of the string."
	},
	{"read-file",			ZREAD_FILE,
	 "Prompts for a path name, with file name completion, and inserts "
	 "the file into the current buffer before the Point. If there is a "
	 "Universal Argument, the current buffer is deleted, first asking to "
	 "save the file if it is modified, before reading in the new "
	 "file. The buffers file name is not changed, any writes will be to "
	 "the old file name."
	},
	{"redisplay",			ZREDISPLAY,
	 "Repaints the entire screen. A Universal Argument causes the command "
	 "to repeat."
	},
	{"replace",			ZREPLACE,
	 "Prompts for a search string and a replacement string and searches "
	 "from the current Point looking for matches of the search "
	 "string. If it finds a match, it replaces the string and "
	 "continues. The Point is left at the position it was in before the "
	 "Query Replace. If the mode Exact is set, the search will be case "
	 "sensitive. A Universal Argument causes the replacement to be done "
	 "globally in all the buffers."
	},
	{"reverse-inc-search",		ZREVERSE_INC_SEARCH,
	 "Searches backwards for the string after every character is entered "
	 "in the PAW. The Delete Previous Character command can be used to "
	 "delete a character and 'forward up' the search. The Newline "
	 "character terminates the search. The Abort command terminates the "
	 "search and places the Point back where it was. Any other command "
	 "terminates the search and performs the command. A Universal "
	 "Argument causes the command to repeat."
	},
	{"reverse-search",		ZREVERSE_SEARCH,
	 "Prompts for a search string and searches from the Point backward "
	 "for a match in the buffer. If a match is found, the Point is moved "
	 "to the start of the match. If the string is not found, then 'Not "
	 "Found' is displayed in the echo window and the Point is not "
	 "moved. The search string is saved and subsequent search and "
	 "replace commands will default to the last search string. If the "
	 "mode Exact is set, the search will be case sensitive. A Universal "
	 "Argument causes the search to look for exactly Arg matches of the "
	 "String Argument."
	},
	{"revert-file",			ZREVERT_FILE,
	 "Rereads the current file. If the file has changed, asks to "
	 "overwrite changes. A Universal Argument causes multiple "
	 "rereads. Mimics the vi 'e' command."
	},
	{"save-all-files",		ZSAVE_ALL_FILES,
	 "Saves all modified buffers. A Universal Argument causes ALL files "
	 "to be saved, modified or not."
	},
	{"save-and-exit",		ZSAVE_AND_EXIT,
	 "An attempt to mimic the vi ZZ command. Does a save file on the "
	 "current buffer and then exits. A universal argument will save all "
	 "files."
	},
	{"save-bindings",		ZSAVE_BINDINGS,
	 "A key bound with the Bind command only stays in effect for the "
	 "duration of the edit session. The Save Bindings command is used to "
	 "permanently save any new bindings in the bindings file. The file "
	 "is saved in the users home directory. A Universal Argument causes "
	 "Arg saves! "
	},
	{"save-file",			ZSAVE_FILE,
	 "Saves the current buffer to its file. If the current buffer has no "
	 "path name, a path name is prompted for (with file name "
	 "completion). A Universal Argument is ignored."
	},
	{"scroll-down",			ZSCROLL_DOWN,
	 "Scrolls the screen down one line. A Universal Argument causes the "
	 "command to scroll multiple lines."
	},
	{"scroll-up",			ZSCROLL_UP,
	 "Scrolls the screen up one line. A Universal Argument causes the "
	 "command to scroll multiple lines."
	},
	{"search",			ZSEARCH,
	 "Prompts for a search string and searches from the Point forward for "
	 "a match in the buffer. If a match is found, the Point is moved to "
	 "the start of the match. If the string is not found, then 'Not "
	 "Found' is displayed in the echo window and the Point is not "
	 "moved. The search string is saved and subsequent search or replace "
	 "commands will default to the last search string. If the mode Exact "
	 "is set, the search will be case sensitive. A Universal Argument "
	 "causes the search to look for exactly Arg matches."
	},
	{"set-bookmark",		ZSET_BOOKMARK,
	 "Places an invisible 'bookmark' at the Point. There are 10 "
	 "bookmarks placed in a ring. The bookmark set is displayed in the "
	 "PAW. A Universal Argument causes the command to repeat."
	},
	{"set-mark",			ZSET_MARK,
	 "Sets the Mark at the Point. A Universal Argument is ignored."
	},
	{"set-variable",		ZSET_VARIABLE,
	 "Allows any of the configurable variables to be set. Command "
	 "completion is supported. Prompts for the variable, then prompts "
	 "for the new setting. A Universal Argument causes numeric or flag "
	 "variables to be set to Arg, string variables ignore the Universal "
	 "Argument."
	},
	{"setenv",			ZSETENV,
	 "Performs a 'setenv' command on an environment variable. The "
	 "variable will keep the setting during the the Zedit session."
	},
	{"show-config",			ZSHOW_CONFIG,
	 "Show the current settings of all the Zedit variables in a "
	 "buffer. The buffer is suitable for use as a .config.z file and can "
	 "be saved to a file using Write File."
	},
	{"size-window",			ZSIZE_WINDOW,
	 "Sets the window to Universal Argument lines."
	},
	{"spell-word",			ZSPELL_WORD,
#if !SPELL
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "Check the spelling of the word at the point. Give hints about "
	 "possible correct spellings."
	},
	{"split-window",		ZSPLIT_WINDOW,
	 "Splits the current window into two windows. The same buffer is "
	 "displayed in both windows and the bottom window is made active. A "
	 "Universal Argument keeps splitting the current window."
	},
	{"stats",			ZSTATS,
	 "Displays some simple stats about Zedit."
	},
	{"swap-chars",			ZSWAP_CHARS,
	 "Swaps the character before the Point with the character at the "
	 "Point. It leaves the Point after the second character. Successive "
	 "commands will 'drag' the character that was before the Point "
	 "towards the end of the line. If the Point was at the end of a line, "
	 "the two characters before the Point are transposed. If the Point is "
	 "at the beginning of the buffer, the character at the Point and the "
	 "character after the Point are transposed. A Universal Argument "
	 "causes the command to repeat."
	},
	{"swap-mark",			ZSWAP_MARK,
	 "The Mark goes to the current position of the Point and the Point is "
	 "set where the Mark was. A Universal Argument is ignored."
	},
	{"swap-words",			ZSWAP_WORDS,
	 "Swaps the words before and after the Point. The Point is left "
	 "after the second word. Successive commands will 'drag' the word "
	 "before the Point towards the end of the line. If the Point is at "
	 "the end of a line, the last word on the line is transposed with "
	 "the first word on the next line. The command does nothing at the "
	 "end of the buffer. If the Point is in the middle of a word, the "
	 "two halves of the word are transposed. A Universal Argument causes "
	 "the command to repeat."
	},
	{"switch-to-buffer",		ZSWITCH_TO_BUFFER,
	 "Prompts for a buffer name to switch to with command completion. "
	 "The previous buffer is stored and displayed as a default. This "
	 "allows quick switching between two buffers. The Again command can "
	 "be used to scroll through the buffer list. A Universal Argument is "
	 "ignored."
	},
	{"tab",				ZTAB,
	 "Handles the tab key. A Universal Argument causes the command to "
	 "repeat."
	},
	{"tag",				ZTAG,
	 "The tag command asks for a function and looks for the function in a "
	 "tagfile. If there is no tagfile specified, then it will prompt for a "
	 "tagfile. A Universal Argument always prompts for a tagfile.\n\n"
	 "Only supports etags style tagfiles."
	},
	{"tag-word",			ZTAG_WORD,
	 "Like the tag command but uses the word at the point for the search tag."
	},
	{"toggle-case",			ZTOGGLE_CASE,
	 "Switches the current buffers minor mode between case sensitive and "
	 "case insensitive for searches and replacements. Ignores the "
	 "Universal Argument."
	},
	{"trim-white-space",		ZTRIM_WHITE_SPACE,
	 "Removes all spaces and tabs on both sides of the Point. The Point "
	 "is left on the character that is to the right of the deleted "
	 "text. The deleted characters are not put in the Kill Buffer. A "
	 "Universal Argument is ignored."
	},
	{"undent",			ZUNDENT,
	 "Removes Universal Argument tabs from the start of every line in "
	 "the region."
	},
	{"undo",			ZUNDO,
#if !UNDO
	 "Note: Disabled in this version of Zedit.\n\n"
#endif
	 "EXPERIMENTAL: Undo the previous edit. There is a list of undos."
	},
	{"unmodify",			ZUNMODIFY,
	 "Turns off the modified flag for the current buffer. Does not change "
	 "the buffer itself. A Universal Argument is ignored."
	},
	{"uppercase-region",		ZUPPERCASE_REGION,
	 "Convert the Region to uppercase. Not allowed in program mode "
	 "buffers. A Universal Argument is ignored."
	},
	{"uppercase-word",		ZUPPERCASE_WORD,
	 "Converts the current word starting at the Point to uppercase. It "
	 "leaves the Point at the end of the word. A Universal Argument "
	 "causes the command to repeat."
	},
	{"view-line",			ZVIEW_LINE,
	 "Makes the Point line the top line in the window. A Universal "
	 "Argument causes the command to repeat."
	},
	{"word-search",			ZWORD_SEARCH,
	 "Search for the word at the point. A Universal Argument causes the "
	 "search to go backwards."
	},
	{"write-file",			ZWRITE_FILE,
	 "Prompts for a path name and writes out the current buffer to this "
	 "file name. Supports file name completion. It changes the path name "
	 "of the current buffer to the new path name. A Universal Argument "
	 "causes only the Region to be written and does not change the name "
	 "of the buffer."
	},
	{"yank",			ZYANK,
	 "Inserts the characters from the Kill Buffer before the Point. The "
	 "characters are inserted, even in overwrite mode. A Universal "
	 "Argument causes the command to repeat."
	},
};
