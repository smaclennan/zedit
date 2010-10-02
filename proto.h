/* proto.h - Zedit function prototypes
 * Copyright (C) 1988-2010 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include "typedefs.h"

/* The Zedit commands */
void Z1wind(void);
void Z2wind(void);
void Zabort(void);
void Zagain(void);
void Zarg(void);
void Zbeauty(void);
void Zbegline(void);
void Zbegwind(void);
void Zbind(void);
void Zbpara(void);
void Zbword(void);
void Zcalc(void);
void Zcapword(void);
void Zcase(void);
void Zcenter(void);
void Zcexpand(void);
void Zcfindexpand(void);
void Zcgoto(void);
void Zcindent(void);
void Zcinsert(void);
void Zcmd(void);
void Zcmdtobuff(void);
void Zcmdbind(void);
void Zcmode(void);
void Zcopyrgn(void);
void Zcount(void);
void Zctrlx(void);
void Zcwd(void);
void Zdate(void);
void Zdelblanks(void);
void Zdelchar(void);
void Zdeleol(void);
void Zdelline(void);
void Zdelrgn(void);
void Zdelwhite(void);
void Zdelwind(void);
void Zdelword(void);
void Zdispbinds(void);
void Zeditfile(void);
void Zempty(void);
void Zendline(void);
void Zendwind(void);
void Zexit(void);
void Zfileread(void);
void Zfilesave(void);
void Zfilewrite(void);
void Zfillchk(void);
void Zfillmode(void);
void Zfillpara(void);
void Zfindfile(void);
void Zfindtag(void);
void Zformtab(void);
void Zfpara(void);
void Zfword(void);
void Zgetbword(void);
void Zgrep(void);
void Zgsearch(void);
void Zgrowwind(void);
void Zhelp(void);
void Zhexout(void);
void Zincsrch(void);
void Zindent(void);
void Zindent(void);
void Zinsert(void);
void Zispace(void);
void Zjoin(void);
void Zkeybind(void);
void Zkill(void);
void Zkillbuff(void);
void Zlgoto(void);
void Zlowregion(void);
void Zlowword(void);
void Zlstbuff(void);
void Zmake(void);
void Zmakedel(void);
void Zman(void);
void Zmeta(void);
void Zmetax(void);
void Zmode(void);
void Zmrkpara(void);
void Znewline(void);
void Znextbuff(void);
void Znextchar(void);
void Znexterr(void);
void Znextline(void);
void Znextpage(void);
void Znextwind(void);
void Znotimpl(void);
void Znxtbookmrk(void);
void Znxtothrwind(void);
void Zopenline(void);
void Zoverin(void);
void Zpart(void);
void Zprevchar(void);
void Zprevline(void);
void Zprevothrwind(void);
void Zprevpage(void);
void Zprevwind(void);
void Zprint(void);
void Zprintpos(void);
void Zquery(void);
void Zquit(void);
void Zquote(void);
void Zrcsci(void);
void Zrcsco(void);
void Zrdelchar(void);
void Zrdelword(void);
void Zredisplay(void);
void Zref(void);
void Zreplace(void);
void Zrereplace(void);
void Zresrch(void);
void Zgresrch(void);
void Zrincsrch(void);
void Zrsearch(void);
void Zsaveall(void);
void Zsavebind(void);
void Zsaveconfig(void);
void Zscrolldown(void);
void Zscrollup(void);
void Zsearch(void);
void Zsetavar(void);
void Zsetbookmrk(void);
void Zsetenv(void);
void Zsetmrk(void);
void Zshell(void);
void Zshrinkwind(void);
void Zsizewind(void);
void Zspell(void);
void Zstat(void);
void Zswapchar(void);
void Zswapmrk(void);
void Zswapword(void);
void Zswitchto(void);
void Ztab(void);
void Ztoend(void);
void Ztostart(void);
void Zundent(void);
void Zunmodf(void);
void Zupregion(void);
void Zupword(void);
void Zviewfile(void);
void Zviewline(void);
void Zvmode(void);
void Zyank(void);
void Zzoom(void);

/* General routines */

#ifdef BSD
int access(char *, int);
#endif
struct passwd *dup_pwent(struct passwd *);
char *AddHome(char*, char*);
void initScrnmarks(void);
struct llist *Add(struct llist**, char*);
char *Addbname(char *);
void Addtocnames(int, char *);
int Ask(char *);
char *Bakname(char *, char *);
int Batoi(void);
int Bcopyrgn(struct mark *, struct buff*);
struct buff *Bcreate(void);
struct mark *Bcremrk(void);
struct page *Bcrepage(struct buff*, struct page *, struct page *, int);
Boolean Bcrsearch(Byte);
Boolean Bcsearch(Byte);
Boolean Bdelbuff(struct buff *);
void Bdelete(unsigned);
void Bdeltomrk(struct mark *);
void Bempty(void);
void Bflush(void);
int Bgetcol(Boolean, int);
void Bgoto(struct buff *);
void Bind(void);
void Binsert(Byte);
void Binstr(char *);
Boolean Bisaftermrk(struct mark *);
Boolean Bisbeforemrk(struct mark *);
int Bitcnt(int);
long Blength(struct buff *);
long Blines(struct buff *);
unsigned long Blocation(unsigned *);
void Blockmove(struct mark *, struct mark *);
int Bmakecol(int, Boolean);
Boolean Bmove(int);
Boolean Bmove1(void);
void Bmrktopnt(struct mark *);
void Boffset(unsigned long);
void Bpnttomrk(struct mark *);
int Breadfile(char *);
Boolean Bsearch(char *, Boolean);
void Bshoveit(void);
void Bswappnt(struct mark *);
void Bswitchto(struct buff *);
void Btoend(void);
void Btostart(void);
int Bwritefd(int);
int Bwritefile(char *);
int Checkpipes(int type);
struct buff *Cfindbuff(char *);
void Clrecho(void);
struct buff *Cmakebuff(char *, char *);
struct buff *Cmdtobuff(char *, char *);
int Cntlines(int);
int Compile(Byte*, Byte*, Byte*);
void Copytomrk(struct mark *);
Boolean CreateRing(void);
void Cswitchto(struct buff *);
Boolean zCursor(void);
Boolean Delay(void);
Boolean Delayprompt(char *);
Boolean Delbname(char *);
void Delbuff(struct buff *);
Boolean Delcmd(void);
Boolean DelcmdAll(void);
char *Dispkey(unsigned, char *);
void Dispit(char *, int *);
void Doincsrch(char *, Boolean);
void Doreplace(int);
void Dotty(void);
void Dline(int);
void Dwait(int);
void Edit(void);
void Execute(void);
void ExtendedLineMarker(void);
int Fileread(char *);
Boolean Filesave(void);
Boolean Findfile(char *, int);
char *Findfirst(char *);
char *Findnext(void);
FILE *Findhelp(int, int, char *);
int Findpath(char *, char *, int, Boolean);
struct wdo *Findwdo(struct buff *);
int Forcecol(void);
void Freelist(struct llist **);
Boolean Getarg(char *, char *, int);
char *Getbtxt(char *, int);
Boolean Getbword(char *, int, int (*)());
int Getdname(char *prompt, char *path);
long Getnum(char *);
int Getplete(char *, char *, char **, int, int);
void Help(int, Boolean);
void Helpit(int);
void Initline(void);
int Innerdsp(int, int, struct mark *);
void Intomem(struct page *);
Boolean Isdir(char *);
Boolean Isext(char *, char *);
Boolean Isfile(char *, char *, char *, Boolean);
Boolean Ispara(char, char);
int Isspace(void);
int Isnotws(void);
int Istoken(void);
int Iswhite(void);
int Isword(void);
void Killtomrk(struct mark *);
char *Lastpart(char *);
char *Limit(char *, int);
void Loadbind(void);
void Loadsaved(void);
void Makecur(struct page *);
void Makeoffset(int);
void Makepaw(char *, Boolean);
void Modeflags(struct wdo *);
void Movepast(int (*pred)(), Boolean forward);
void Moveto(int (*pred)(), Boolean forward);
Boolean Mrkaftermrk(struct mark *, struct mark *);
Boolean Mrkatmrk(struct mark *, struct mark *);
Boolean Mrkbeforemrk(struct mark *, struct mark *);
void Mshow(unsigned);
Boolean Mv(char *, char *);
Boolean Cp(char *, char *);
int main(int, char **);
void Newtitle(char *);
char *Nocase(char *);
void NoMem(void);
int Parse(char *);
void parsem(char *, Boolean);
int Pathfixup(char *, char *);
Boolean Pcmdplete(Boolean);
void Pinsert(void);
int PipeToBuff(struct buff *buff, char *instr);
int BuffToPipe(struct buff *buff, char *instr);
void Pnewline(void);
void Pntmove(int, int);
void Pout(char *, Boolean);
void Pset(int, int, int);
int Prefline(void);
void PutPaw(char *, int);
void ReadVfile(void);
Boolean Readone(char *, char *);
void Redisplay(void);
void Reframe(void);
void Refresh(void);
void Regerr(int);
Boolean Resize(int);
void Save(struct buff *);
Boolean Saveall(Boolean);
void Setavar(char *, Boolean);
char *Setmodes(struct buff *);
int Settabsize(unsigned);
void Setup(int, char **);
Boolean Step(Byte *);
void SetMark(Boolean);
char *Strstr(char *, char *);
char *Strup(char *);
void Syerr(int);
void Tbell(void);
void Tcleol(void);
void Tclrwind(void);
void Termsize(void);
void Tfini(void);
#ifdef XWINDOWS
void Tflush(void);
#else
void Tforce(void);
#endif
int Tgetcmd(void);
Byte Tgetkb(void);
void Tgoto(int, int);
void Tindent(int);
#ifdef XWINDOWS
void Tinit(int, char **);
#else
void Tinit(void);
#endif
void Titot(unsigned);
int Tkbrdy(void);
void Tobegline(void);
void Toendline(void);
int Tolower(int);
int Toupper(int);
void Toggle_mode(int);
void Tprntchar(Byte);
void Tprntstr(char *);
void Tsize(int *, int *);
void Tstyle(int);
void Tungetkb(void);
void Unmark(struct mark *);
void Usage(char *);
void Varval(int var);
void Vsetmod(Boolean);
void Vsetmrk(struct mark *);
void Walign(struct buff *);
int Width(Byte, int, Boolean);
void Wload(char *, int, int, unsigned long, int);
int Write_rgn(char *);
void free_pwent(struct passwd *pw);
void Hangup(int);
Boolean notdup_key(int k);
void Quit(void);

/* Terminal driver specific routines */


#ifndef XWINDOWS
void TIinit(void);
void TIfini(void);
#endif


/* compile switched routines */


void Message(struct buff *, char *);
void PrintExit(int code);

int Dopipe(struct buff *, char *);
void Winit(void);

#ifdef PIPESH
int Checkpipes(int);
Boolean Doshell(void);
Boolean Invoke(struct buff *, char **);
int Readpipes(fd_set *);
void Sendtopipe(void);
void Sigchild(int);
char *Wordit(char **);
#endif

int Isalpha(void);

void Unvoke(struct buff *, Boolean);

void Zmail(void);


void Dbg(char *fmt, ...);
void Dbgname(char *);
void Fcheck(void);


/* for getfname */

int Getfname(char *, char *);
void Zfname(void);
struct llist *GetFill(char *, char **, int *, Boolean*);
struct llist *Fill_list(char *);
int nmatch(char *, char *);


#ifdef SPELL
void sreplace(char *);
#endif

void Wswitchto(struct wdo *wdo);
void Winvalid(struct wdo *wdo);
void Wsize(void);
Boolean WuseOther(char *);

#ifdef MEMLOG
void loginit(char *);
void logfini(void);
char *logmalloc(unsigned, char *, unsigned);
char *logdup(char *, char *, unsigned);
void logfree(char *, char *, unsigned);

#define malloc(n)		logmalloc((n), __FILE__, __LINE__)
#define strdup(m)		logdup((m), __FILE__, __LINE__)
#define free(m)			logfree((m), __FILE__, __LINE__)
#endif

void KillHelp(void);

#ifdef XWINDOWS
char *KeyToName(int, char *);
void ShowCursor(Boolean);
void ShowMark(Boolean);
int Pinvoke(char *argv[], FILE **in, FILE **out);
int ColorResource(char *name, char *class, int *pixel);
void Tputchar(char);
int GetColor(char *, int *);
int GetXColor(char *, XColor *);
XFontStruct *LoadFonts(void);
void Xinit(char *app, int *argc, char **argv);
void XShellInput(void);

void PopupSearch(void);
char GetQueryCmd(char prev);
void QueryDone(void);

void xusage(void);

void audioExit(void);

void GrabKeyboard();
void Xfindtag(void);
#endif

#if COMMENTBOLD
void AddComment(void);
void ResetComments(void);
void CheckComment(void);
void Recomment(void);
void AddCPP(void);
#endif

#ifdef SCROLLBARS
void UpdateScrollbars();
#endif

void undo_add(int size);
void undo_del(int size);
void undo_clear(struct buff *buff);
void Zundo(void);
