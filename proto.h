/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/

/* The Zedit commands */
ZPROC(Z1wind)
ZPROC(Z2wind)
ZPROC(Zabort)
ZPROC(Zagain)
ZPROC(Zarg)
ZPROC(Zbeauty)
ZPROC(Zbegline)
ZPROC(Zbegwind)
ZPROC(Zbind)
ZPROC(Zbpara)
ZPROC(Zbword)
ZPROC(Zcalc)
ZPROC(Zcapword)
ZPROC(Zcase)
ZPROC(Zcenter)
ZPROC(Zcexpand)
ZPROC(Zcfindexpand)
ZPROC(Zcgoto)
ZPROC(Zcindent)
ZPROC(Zcinsert)
ZPROC(Zcmd)
ZPROC(Zcmdtobuff)
ZPROC(Zcmdbind)
ZPROC(Zcmode)
ZPROC(Zcopyrgn)
ZPROC(Zcount)
ZPROC(Zctrlx)
ZPROC(Zcwd)
ZPROC(Zdate)
ZPROC(Zdelblanks)
ZPROC(Zdelchar)
ZPROC(Zdeleol)
ZPROC(Zdelline)
ZPROC(Zdelrgn)
ZPROC(Zdelwhite)
ZPROC(Zdelwind)
ZPROC(Zdelword)
ZPROC(Zdispbinds)
ZPROC(Zeditfile)
ZPROC(Zempty)
ZPROC(Zendline)
ZPROC(Zendwind)
ZPROC(Zexit)
ZPROC(Zfileread)
ZPROC(Zfilesave)
ZPROC(Zfilewrite)
ZPROC(Zfillchk)
ZPROC(Zfillmode)
ZPROC(Zfillpara)
ZPROC(Zfindfile)
ZPROC(Zfindtag)
ZPROC(Zformtab)
ZPROC(Zfpara)
ZPROC(Zfword)
ZPROC(Zgetbword)
ZPROC(Zgrep)
ZPROC(Zgsearch)
ZPROC(Zgrowwind)
ZPROC(Zhelp)
ZPROC(Zhexout)
ZPROC(Zincsrch)
ZPROC(Zindent)
ZPROC(Zindent)
ZPROC(Zinsert)
ZPROC(Zispace)
ZPROC(Zjoin)
ZPROC(Zkeybind)
ZPROC(Zkill)
ZPROC(Zkillbuff)
ZPROC(Zlgoto)
ZPROC(Zlowregion)
ZPROC(Zlowword)
ZPROC(Zlstbuff)
ZPROC(Zmake)
ZPROC(Zmakedel)
ZPROC(Zman)
ZPROC(Zmeta)
ZPROC(Zmetax)
ZPROC(Zmode)
ZPROC(Zmrkpara)
ZPROC(Znewline)
ZPROC(Znextbuff)
ZPROC(Znextchar)
ZPROC(Znexterr)
ZPROC(Znextline)
ZPROC(Znextpage)
ZPROC(Znextwind)
ZPROC(Znotimpl)
ZPROC(Znxtbookmrk)
ZPROC(Znxtothrwind)
ZPROC(Zopenline)
ZPROC(Zoverin)
ZPROC(Zpart)
ZPROC(Zprevchar)
ZPROC(Zprevline)
ZPROC(Zprevothrwind)
ZPROC(Zprevpage)
ZPROC(Zprevwind)
ZPROC(Zprint)
ZPROC(Zprintpos)
ZPROC(Zquery)
ZPROC(Zquit)
ZPROC(Zquote)
ZPROC(Zrcsci)
ZPROC(Zrcsco)
ZPROC(Zrdelchar)
ZPROC(Zrdelword)
ZPROC(Zredisplay)
ZPROC(Zref)
ZPROC(Zreplace)
ZPROC(Zrereplace)
ZPROC(Zresrch)
ZPROC(Zgresrch)
ZPROC(Zrincsrch)
ZPROC(Zrsearch)
ZPROC(Zsaveall)
ZPROC(Zsavebind)
ZPROC(Zsaveconfig)
ZPROC(Zscrolldown)
ZPROC(Zscrollup)
ZPROC(Zsearch)
ZPROC(Zsetavar)
ZPROC(Zsetbookmrk)
ZPROC(Zsetenv)
ZPROC(Zsetmrk)
ZPROC(Zshell)
ZPROC(Zshrinkwind)
ZPROC(Zsizewind)
ZPROC(Zspell)
ZPROC(Zstat)
ZPROC(Zswapchar)
ZPROC(Zswapmrk)
ZPROC(Zswapword)
ZPROC(Zswitchto)
ZPROC(Ztab)
ZPROC(Ztoend)
ZPROC(Ztostart)
ZPROC(Zundent)
ZPROC(Zunmodf)
ZPROC(Zupregion)
ZPROC(Zupword)
ZPROC(Zviewfile)
ZPROC(Zviewline)
ZPROC(Zvmode)
ZPROC(Zyank)
ZPROC(Zzoom)

/* General routines */

#if BSD
int access ARGS((char *, int));
#endif
struct passwd *dup_pwent ARGS((struct passwd *));
char *AddHome ARGS((char*, char*));
void initScrnmarks NOARGS;
struct llist *Add ARGS((struct llist**, char*));
char *Addbname ARGS((char*));
void Addtocnames ARGS((int, char*));
int Ask ARGS((char*));
char *Bakname(char*, char*);
int Batoi NOARGS;
int Bcopyrgn ARGS((Mark*, Buffer*));
Buffer *Bcreate NOARGS;
Mark *Bcremrk NOARGS;
Page *Bcrepage ARGS((Buffer*, Page*, Page*, int));
Boolean Bcrsearch ARGS((Byte));
Boolean Bcsearch ARGS((Byte));
Boolean Bdelbuff ARGS((Buffer*));
void Bdelete ARGS((unsigned));
void Bdeltomrk ARGS((Mark*));
void Bempty NOARGS;
void Bflush NOARGS;
int Bgetcol ARGS((Boolean, int));
void Bgoto ARGS((Buffer *));
void Bind NOARGS;
void Binsert ARGS((Byte));
void Binstr ARGS((char*));
Boolean Bisaftermrk ARGS((Mark*));
Boolean Bisbeforemrk ARGS((Mark*));
int Bitcnt ARGS((int));
long Blength ARGS((Buffer*));
long Blines ARGS((Buffer*));
unsigned long Blocation ARGS((unsigned*));
void Blockmove ARGS((Mark*, Mark*));
int Bmakecol ARGS((int, Boolean));
Boolean Bmove ARGS((int));
Boolean Bmove1 ARGS((void));
void Bmrktopnt ARGS((Mark*));
void Boffset ARGS((unsigned long));
void Bpnttomrk ARGS((Mark*));
int Breadfile ARGS((char*));
Boolean Bsearch ARGS((char*, Boolean));
void Bshoveit NOARGS;
void Bswappnt ARGS((Mark*));
void Bswitchto ARGS((Buffer*));
void Btoend NOARGS;
void Btostart NOARGS;
int Bwritefd ARGS((int));
int Bwritefile ARGS((char*));
int Checkpipes ARGS((int type));
Buffer *Cfindbuff ARGS((char*));
void Clrecho NOARGS;
Buffer *Cmakebuff ARGS((char*, char*));
Buffer *Cmdtobuff ARGS((char*, char*));
int Cntlines ARGS((int));
int Compile ARGS((Byte*, Byte*, Byte*));
void Copytomrk ARGS((Mark*));
Boolean CreateRing NOARGS;
void Cswitchto ARGS((Buffer*));
Boolean zCursor NOARGS;
Boolean Delay NOARGS;
Boolean Delayprompt ARGS((char*));
Boolean Delbname ARGS((char*));
Proc Delbuff ARGS((Buffer *));
Boolean Delcmd NOARGS;
Boolean DelcmdAll NOARGS;
char *Dispkey ARGS((unsigned, char*));
void Dispit ARGS((char*, int*));
void Doincsrch ARGS((char*, Boolean));
void Doreplace ARGS((int));
Proc Dotty NOARGS;
void Dline ARGS((int));
void Dwait ARGS((int));
void Edit NOARGS;
void Execute NOARGS;
void ExtendedLineMarker NOARGS;
int Fileread ARGS((char*));
Boolean Filesave NOARGS;
Boolean Findfile ARGS((char*, int));
char *Findfirst ARGS((char*));
char *Findnext NOARGS;
FILE *Findhelp ARGS((int, int, char *));
int Findpath ARGS((char*, char*, int, Boolean));
WDO *Findwdo ARGS((Buffer *));
int Forcecol NOARGS;
void Freelist ARGS((struct llist**));
Boolean Getarg ARGS((char*, char*, int));
char *Getbtxt ARGS((char*, int));
Boolean Getbword ARGS((char*, int, int (*)()));
int Getdname ARGS((char *prompt, char *path));
long Getnum ARGS((char*));
int Getplete ARGS((char*, char*, char **, int, int));
void Help ARGS((int, Boolean));
void Helpit ARGS((int));
void Initline NOARGS;
int Innerdsp ARGS((int, int, Mark*));
void Intomem ARGS((Page*));
Boolean Isdir ARGS((char*));
Boolean Isext ARGS((char*, char*));
Boolean Isfile ARGS((char*, char*, char*, Boolean));
Boolean Ispara ARGS((char, char));
int Isspace NOARGS;
int Isnotws NOARGS;
int Istoken NOARGS;
int Iswhite NOARGS;
int Isword NOARGS;
void Killtomrk ARGS((Mark*));
char *Lastpart ARGS((char*));
char *Limit ARGS((char*, int));
void Loadbind NOARGS;
void Loadsaved NOARGS;
void Loadwdo ARGS((char *));
void Makecur ARGS((Page*));
void Makeoffset ARGS((int));
void Makepaw ARGS((char*, Boolean));
void Modeflags ARGS((WDO *));
void Movepast ARGS((int (*pred)(), Boolean forward));
void Moveto ARGS((int (*pred)(), Boolean forward));
Boolean Mrkaftermrk ARGS((Mark*, Mark*));
Boolean Mrkatmrk ARGS((Mark*, Mark*));
Boolean Mrkbeforemrk ARGS((Mark*, Mark*));
Proc Mshow ARGS((unsigned));
Boolean Mv ARGS((char*, char*));
Boolean Cp ARGS((char*, char*));
int main ARGS((int, char**));
void Newtitle ARGS((char *));
char *Nocase ARGS((char*));
void NoMem NOARGS;
int Parse ARGS((char*));
void parsem ARGS((char*, Boolean));
int Pathfixup ARGS((char*, char*));
Boolean Pcmdplete ARGS((Boolean));
Proc Pinsert NOARGS;
int PipeToBuff ARGS((Buffer *buff, char *instr));
int BuffToPipe ARGS((Buffer *buff, char *instr));
Proc Pnewline NOARGS;
void Pntmove ARGS((int, int));
void Pout ARGS((char *, Boolean));
void Pset ARGS((int, int, int));
int Prefline NOARGS;
void PutPaw ARGS((char*, int));
void ReadVfile NOARGS;
Boolean Readone ARGS((char*, char*));
void Redisplay NOARGS;
void Reframe NOARGS;
void Refresh NOARGS;
void Regerr ARGS((int));
Boolean Resize ARGS((int));
void Save ARGS((Buffer*));
Boolean Saveall ARGS((Boolean));
void Setavar ARGS((char*, Boolean));
char *Setmodes ARGS((Buffer *));
int Settabsize ARGS((unsigned));
void Setup ARGS((int, char**));
Boolean Step ARGS((Byte*));
void SetMark ARGS((Boolean));
char *Strstr ARGS((char*, char*));
char *Strup ARGS((char*));
void Syerr ARGS((int));
void Tbell NOARGS;
void Tcleol NOARGS;
void Tclrwind NOARGS;
void Termsize NOARGS;
void Tfini NOARGS;
#if XWINDOWS
void Tflush NOARGS;
#endif
void Tforce NOARGS;
int Tgetcmd NOARGS;
Byte Tgetkb NOARGS;
void Tgoto ARGS((int, int));
void Tindent ARGS((int));
#if XWINDOWS
void Tinit ARGS((int, char**));
#else
void Tinit NOARGS;
#endif
void Titot ARGS((unsigned));
int Tkbrdy NOARGS;
void Tobegline NOARGS;
void Toendline NOARGS;
int Tolower ARGS((int));
int Toupper ARGS((int));
void Toggle_mode ARGS((int));
void Tprntchar ARGS((Byte));
void Tprntstr ARGS((char*));
void Tsize ARGS((int*, int*));
void Tstyle ARGS((int));
void Tungetkb NOARGS;
void Unmark ARGS((Mark*));
void Usage ARGS((char*));
void Varval ARGS((struct avar*));
void Vsetmod ARGS((Boolean));
void Vsetmrk ARGS((Mark*));
void Walign ARGS((Buffer*));
int Width ARGS((Byte, int, Boolean));
void Wload ARGS((char *, int, int, unsigned long, int));
int Write_rgn ARGS((char *));
void free_pwent ARGS((struct passwd *pw));
void Hangup ARGS((int));
Boolean notdup_key(int k);


/* Terminal driver specific routines */


#if TERMINFO || ANSI
void TIinit NOARGS;
void TIfini NOARGS;
#endif


/* compile switched routines */


void Message ARGS((Buffer *, char *));
void PrintExit ARGS((int code));

int Dopipe ARGS((Buffer*, char*));
void Winit NOARGS;

#if PIPESH
int Checkpipes ARGS((int));
Boolean Doshell NOARGS;
Boolean Invoke ARGS((Buffer*, char **));
int Readpipes ARGS((fd_set *));
void Sendtopipe NOARGS;
void Sigchild ARGS((int));
char *Wordit ARGS((char**));
#endif

int Isalpha NOARGS;

void Unvoke ARGS((Buffer*, Boolean));

Proc Zmail NOARGS;


#if DBG
Boolean Dbgname ARGS((char*));
void Dbgsig ARGS((int));
void Fcheck NOARGS;

char *XEventName ARGS((int type));
#endif


/* for getfname */

int Getfname ARGS((char*, char*));
Proc Zfname NOARGS;
Proc Zmatch NOARGS;
struct llist *GetFill ARGS((char*, char**, int*, Boolean*));
struct llist *Fill_list ARGS((char*));
int nmatch ARGS((char*, char*));


#ifdef SPELL
void sreplace ARGS((char *));
#endif

void Wswitchto ARGS((WDO *wdo));
void Winvalid ARGS((WDO *wdo));
void Wsize NOARGS;
Boolean WuseOther ARGS((char *));

#ifdef MEMLOG
void loginit ARGS((char*));
void logfini NOARGS;
char *logmalloc ARGS((unsigned, char*, unsigned));
char *logdup ARGS((char*, char*, unsigned));
void logfree ARGS((char*, char*, unsigned));

#define malloc(n)		logmalloc((n), __FILE__, __LINE__)
#define strdup(m)		logdup((m), __FILE__, __LINE__)
#define free(m)			logfree((m), __FILE__, __LINE__)
#endif

void KillHelp NOARGS;

#if XWINDOWS
char *KeyToName ARGS((int, char*));
void ShowCursor ARGS((Boolean));
void ShowMark ARGS((Boolean));
int Pinvoke ARGS((char *argv[], FILE **in, FILE **out));
int ColorResource ARGS((char *name, char *class, int *pixel));
void Tputchar ARGS((char));
int GetColor ARGS((char*, int*));
int GetXColor ARGS((char*, void*));
void LoadFonts NOARGS;
void Xinit ARGS((char *app, int *argc, char **argv));
void XShellInput NOARGS;

void PopupSearch NOARGS;
char GetQueryCmd ARGS((char prev));
void QueryDone   NOARGS;

void xusage NOARGS;

int StartProg ARGS((char *prog));
void XDeleteBuffer ARGS((char *bname));
void ProcessFDs NOARGS;
void CleanupSocket ARGS((int i));
void closeSockets NOARGS;
void audioExit NOARGS;

void GrabKeyboard();
void Xfindtag NOARGS;
#endif

#if COMMENTBOLD
void AddComment NOARGS;
void ResetComments NOARGS;
void CheckComment NOARGS;
void Recomment NOARGS;
void AddCPP NOARGS;
#endif

#ifdef SCROLLBARS
void UpdateScrollbars();
#endif

void undo_add(int size);
void undo_del(int size);
void undo_clear(Buffer *buff);
Proc Zundo(void);
