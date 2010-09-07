/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include "keys.h"

#if XWINDOWS
char *KeyNames[] = {
 	"Home",		"Left",	"Up",		"Right",	"Down",		"Prior",
	"Next",		"End",	"Begin",	"Select",	"Print",	"Execute",
	"Insert", 	"???",	"Undo",		"Redo",		"Menu",		"Find",	
 	"Cancel",	"Help",	"Break",	"F1",		"F2",		"F3",	
 	"F4",		"F5",	"F6",		"F7",		"F8",		"F9",	
 	"F10",		"F11",	"F12",		"F13",		"F14",		"F15",	
 	"F16",		"F17",	"F18",		"F19",		"F20",		"F21",	
 	"F22",		"F23",	"F24",		"F25",		"F26",		"F27",	
 	"F28",		"F29",	"F30",		"F31",		"F32",		"F33",	
 	"F34",		"F35",	"Ctrl Home","Ctrl Left","Ctrl Up",	"Ctrl Right",
 	"Ctrl Down","Ctrl Prior", 		"Ctrl Next","Ctrl End",	"Ctrl Begin"
};
#endif

char *Dispkey( key, s )
unsigned key;
char *s;
{
	char *p;
	int j;

	*s = '\0';
#if TERMINFO || TERMCAP
	if( key > SPECIAL_START )
		return strcpy(s, Tkeys[key - SPECIAL_START].label);
	if( key > 127 ) strcpy( s, key < 256 ? "M-" : "C-X " );
#endif
#if XWINDOWS
	if(key >= ZXK_START && key < NUMKEYS)
		return strcpy(s, KeyNames[key - ZXK_START]);
#endif
	if( (j = key & 0x7f) == 27 )
		strcat( s, "ESC" );
	else if( j < 32 || j == 127 )
	{
		strcat( s, "C-" );
		p = s + strlen( s );
		*p++ = j ^ '@';
		*p = '\0';
	}
	else if( j == 32 )
		strcat( s, "Space" );
	else
	{
		p = s + strlen( s );
		*p++ = j;
		*p = '\0';
	}
	return( s );
}
