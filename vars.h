/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
/* The variable structure.
 */
struct avar
{
	char *vname;
	Word vtype;
	Word val;
};
extern struct avar Vars[];


/* var defines */
#define VADDNL				0
#define VASCHAR				VADDNL + 1
#define VASEXTS				VASCHAR + 1
#define VBACKUP				VASEXTS + 1
#define VCEXTS				VBACKUP + 1
#define VCLEAR				VCEXTS + 1
#define VCTABS				VCLEAR + 1
#define VDATESTR			VCTABS + 1
#define VDOSAVE				VDATESTR + 1
#define VEXACT				VDOSAVE + 1
#define VEXECONMATCH		VEXACT + 1
#define VEXPAND				VEXECONMATCH + 1
#define VFILLWIDTH			VEXPAND + 1
#define VFLOW				VFILLWIDTH + 1
#define VFONT				VFLOW + 1
#define VGREP				VFONT + 1
#define VKILLLINE			VGREP + 1
#define VLINES				VKILLLINE + 1
#define VMACROFILE			VLINES + 1
#define VMAIL				VMACROFILE + 1
#define VMAKE				VMAIL + 1
#define VMARGIN				VMAKE + 1
#define VMATCH				VMARGIN + 1
#define VNORMAL				VMATCH + 1
#define VOVWRT				VNORMAL + 1
#define VPOPMAKE			VOVWRT + 1
#define VPOPMAN				VPOPMAKE + 1
#define VPRINT				VPOPMAN + 1
#define VSAVE				VPRINT + 1
#define VSHOWCWD			VSAVE + 1
#define VSILENT				VSHOWCWD + 1
#define VSINGLE				VSILENT + 1
#define VSPACETAB           VSINGLE + 1
#define VTABS				VSPACETAB + 1
#define VTAG				VTABS + 1
#define VSEXTS				VTAG + 1
#define VTEXTS				VSEXTS + 1
#define VUSEOTHER			VTEXTS + 1
#define VVISBELL			VUSEOTHER + 1
#define VXORCURSOR			VVISBELL + 1
#define VARNUM				VXORCURSOR + 1


/* Var macros */
#define Fillwidth()			(Vars[ VFILLWIDTH ].val)
#define Margin()			(Vars[ VMARGIN ].val)


/* the variable types */
#define STRING		0
#define DECIMAL		1
#define FLAG		2
#define NUMERRSTR	2

/* only include this in vars.c */
#ifdef INCLUDE_VARS_STRUCT

#if 1
/* hp9000 must be this way!
 * I cannot remember what needed the
 * other way!
 */
#define VALUE(n)	n
#else
#define VALUE(n)	{n}
#endif

/* NOTE: Spaces not allowed in variable names!!! */
struct avar Vars[] =
{
	{"AddNL",		FLAG,		VALUE(1)},
	{"AsmChar",		STRING,		VALUE(0)},
	{"AsmExtends",	STRING,		VALUE(0)},
	{"Backup",		FLAG,		VALUE(1)},
	{"CExtends",	STRING,		VALUE(0)},
	{"ClearExit",	FLAG,		VALUE(0)},
	{"CTabs",		DECIMAL,	VALUE(4)},
	{"DateFormat",	STRING,		VALUE(0)},
	{"DoSave",		FLAG,		VALUE(0)},
	{"Exact",		FLAG,		VALUE(1)},
	{"ExecOnMatch",	FLAG,		VALUE(0)},
	{"ExpandPaths",	FLAG,		VALUE(1)},
	{"FillWidth",	DECIMAL,	VALUE(72)},
	{"FlowControl",	FLAG,		VALUE(0)},
	{"Font",		STRING,		VALUE(0)},
	{"Grep",	    STRING,		VALUE(0)},
	{"KillLine",    FLAG,		VALUE(1)},
	{"Lines",		FLAG,		VALUE(1)},
	{"Macrofile",	STRING,		VALUE(0)},
	{"MailCmd",		STRING,		VALUE(0)},
	{"MakeCmd",		STRING,		VALUE(0)},
	{"Margin",		DECIMAL,	VALUE(0)},
	{"Match",		DECIMAL,	VALUE(1)},
	{"Normal",		FLAG,		VALUE(1)},
	{"Overwrite",	FLAG,		VALUE(0)},
	{"PopupMake",	FLAG,		VALUE(0)},
	{"PopupMan",	FLAG,		VALUE(0)},
	{"Print",		STRING,		VALUE(0)},
	{"SaveOnExit",	FLAG,		VALUE(0)},
	{"ShowCWD",		FLAG,		VALUE(1)},
	{"Silent",		FLAG,		VALUE(0)},
	{"SingleScroll",FLAG,		VALUE(1)},
	{"SpaceTabs",	FLAG,       VALUE(0)},
	{"Tabs",		DECIMAL,	VALUE(8)},
	{"Tagfile",		STRING,		VALUE(0)},
	{"TCLExtends",	STRING,		VALUE(0)},
	{"TExtends",	STRING,		VALUE(0)},
	{"UseOtherWdo",	FLAG,		VALUE(0)},
	{"VisibleBell", FLAG,		VALUE(1)},
	{"XorCursor",	FLAG,		VALUE(XORCURSOR)},
};
#define VARSIZE		sizeof( struct avar )
#define NUMVARS		(sizeof(Vars) / VARSIZE)
#endif
