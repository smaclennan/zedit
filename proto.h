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
void Zrevertfile(void);
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
void Zmail(void);
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
void Zundo(void);
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
void initscrnmarks(void);
int ask(char *);
int batoi(void);
int bcopyrgn(struct mark *, struct buff*);
struct buff *bcreate(void);
struct mark *bcremrk(void);
Boolean bcrsearch(Byte);
Boolean bcsearch(Byte);
Boolean bdelbuff(struct buff *);
void bdelete(unsigned);
void bdeltomrk(struct mark *);
void bempty(void);
int bgetcol(Boolean, int);
void bgoto(struct buff *);
void bind(void);
void binsert(Byte);
void binstr(char *);
Boolean bisaftermrk(struct mark *);
Boolean bisbeforemrk(struct mark *);
long blength(struct buff *);
long blines(struct buff *);
unsigned long blocation(unsigned *);
int bmakecol(int, Boolean);
Boolean bmove(int);
Boolean bmove1(void);
void bmrktopnt(struct mark *);
void boffset(unsigned long);
void bpnttomrk(struct mark *);
int breadfile(char *);
Boolean bstrsearch(char *, Boolean);
void bshoveit(void);
void bswappnt(struct mark *);
void bswitchto(struct buff *);
void btoend(void);
void btostart(void);
int bwritefd(int);
int bwritefile(char *);
int checkpipes(int type);
struct buff *cfindbuff(char *);
struct buff *cmakebuff(char *, char *);
struct buff *cmdtobuff(char *, char *);
int cntlines(int);
int compile(Byte*, Byte*, Byte*);
void cswitchto(struct buff *);
Boolean delay(void);
Boolean delayprompt(char *);
Boolean delbname(char *);
void delbuff(struct buff *);
Boolean delcmd(void);
Boolean delcmdall(void);
char *dispkey(unsigned, char *);
void execute(void);
void extendedlinemarker(void);
Boolean filesave(void);
Boolean findfile(char *, int);
int findpath(char *, char *, int, Boolean);
struct wdo *findwdo(struct buff *);
Boolean getarg(char *, char *, int);
char *getbtxt(char *, int);
Boolean getbword(char *, int, int (*)());
int getdname(char *prompt, char *path);
int getplete(char *, char *, char **, int, int);
void initline(void);
Boolean isdir(char *);
Boolean isfile(char *, char *, char *, Boolean);
Boolean Ispara(char, char);
int bisspace(void);
int bistoken(void);
int biswhite(void);
int bisword(void);
void killtomrk(struct mark *);
char *lastpart(char *);
char *limit(char *, int);
void loadbind(void);
void loadsaved(void);
void makecur(struct page *);
void makeoffset(int);
void makepaw(char *, Boolean);
void modeflags(struct wdo *);
void movepast(int (*pred)(), Boolean forward);
void moveto(int (*pred)(), Boolean forward);
Boolean mrkaftermrk(struct mark *, struct mark *);
Boolean mrkatmrk(struct mark *, struct mark *);
Boolean mrkbeforemrk(struct mark *, struct mark *);
void newtitle(char *);
char *nocase(char *);
void parsem(char *, Boolean);
int pathfixup(char *, char *);
void pinsert(void);
void pnewline(void);
void pntmove(int, int);
void pout(char *, Boolean);
void pset(int, int, int);
int prefline(void);
void putpaw(char *, int);
void readvfile(void);
Boolean readone(char *, char *);
void redisplay(void);
void reframe(void);
void refresh(void);
void regerr(int);
Boolean paw_resize(int);
void save(struct buff *);
Boolean saveall(Boolean);
char *setmodes(struct buff *);
int settabsize(unsigned);
Boolean step(Byte *);
void setmark(Boolean);
char *strup(char *);
char *stristr(char *str1, char *str2);
void tbell(void);
void tcleol(void);
void tclrwind(void);
void termsize(void);
void tfini(void);
#ifdef XWINDOWS
void tflush(void);
#else
void tforce(void);
#endif
int tgetcmd(void);
Byte tgetkb(void);
void tgoto(int, int);
void tindent(int);
#ifdef XWINDOWS
void tinit(int, char **);
#else
void tinit(void);
#endif
void titot(unsigned);
int tkbrdy(void);
void tobegline(void);
void toendline(void);
void toggle_mode(int);
void tprntchar(Byte);
void tprntstr(char *);
void tsize(int *, int *);
void tstyle(int);
void unmark(struct mark *);
void varval(int var);
void vsetmod(Boolean);
void vsetmrk(struct mark *);
int chwidth(Byte, int, Boolean);
void wload(char *, int, int, unsigned long, int);
void free_pwent(struct passwd *pw);
void hangup(int);
Boolean notdup_key(int k);
void quit(void);

/* Terminal driver specific routines */
#ifndef XWINDOWS
void tlinit(void);
void tlfini(void);
#endif

/* compile switched routines */
void message(struct buff *, char *);

int dopipe(struct buff *, char *);
void winit(void);

#ifdef PIPESH
int checkpipes(int);
Boolean doshell(void);
Boolean invoke(struct buff *, char **);
int readpipes(fd_set *);
void sendtopipe(void);
void sigchild(int);
#endif

void unvoke(struct buff *, Boolean);



void Dbg(char *fmt, ...);
void Dbgname(char *);
void fcheck(void);


/* for getfname */

int getfname(char *, char *);
void Zfname(void);
int nmatch(char *, char *);


#ifdef SPELL
void sreplace(char *);
#endif

void wswitchto(struct wdo *wdo);
void wsize(void);
Boolean wuseother(char *);

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

#if COMMENTBOLD
void addcomment(void);
void resetcomments(void);
void checkcomment(void);
void recomment(void);
void addcpp(void);
#else
static inline void addcomment(void) {}
static inline void resetcomments(void) {}
static inline void checkcomment(void) {}
static inline void recomment(void) {}
static inline void addcpp(void) {}
#endif

#ifdef SCROLLBARS
void updatescrollbars();
#endif

void undo_add(int size);
void undo_del(int size);
void undo_clear(struct buff *buff);

/* Only exported for X */
void copytomrk(struct mark *tmark);
int innerdsp(int from, int to, struct mark *pmark);
