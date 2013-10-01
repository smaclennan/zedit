/* proto.h - Zedit function prototypes
 * Copyright (C) 1988-2013 Sean MacLennan
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

#include "typedefs.h"

#ifdef FCHECK
#define Z(f) static void f(void) {}
#else
#define Z(f) void f(void)
#endif

/* The Zedit commands */
Z(Z1wind);
Z(Z2wind);
Z(Zabort);
Z(Zagain);
Z(Zarg);
Z(Zbegline);
Z(Zbegwind);
Z(Zbind);
Z(Zbpara);
Z(Zbword);
Z(Zcalc);
Z(Zcapword);
Z(Zcase);
Z(Zcenter);
Z(Zcexpand);
Z(Zcfindexpand);
Z(Zcgoto);
Z(Zcindent);
Z(Zcinsert);
Z(Zcmd);
Z(Zcmdtobuff);
Z(Zcmdbind);
Z(Zcmode);
Z(Zcopyrgn);
Z(Zcount);
Z(Zctrlx);
Z(Zcwd);
Z(Zdelblanks);
Z(Zdelchar);
Z(Zdeleol);
Z(Zdelline);
Z(Zdelrgn);
Z(Zdelwhite);
Z(Zdelwind);
Z(Zdelword);
Z(Zdispbinds);
Z(Zrevertfile);
Z(Zempty);
Z(Zendline);
Z(Zendwind);
Z(Zexit);
Z(Zfileread);
Z(Zfilesave);
Z(Zfilewrite);
Z(Zfillchk);
Z(Zfillmode);
Z(Zfillpara);
Z(Zfindfile);
Z(Zfindtag);
Z(Zfname);
Z(Zfpara);
Z(Zfword);
Z(Zgetbword);
Z(Zgrep);
Z(Zgsearch);
Z(Zgrowwind);
Z(Zhelp);
Z(Zhexout);
Z(Zincsrch);
Z(Zindent);
Z(Zinsert);
Z(Zjoin);
Z(Zkeybind);
Z(Zkill);
Z(Zkillbuff);
Z(Zlgoto);
Z(Zlowregion);
Z(Zlowword);
Z(Zlstbuff);
Z(Zmake);
Z(Zmakedel);
Z(Zmeta);
Z(Zmetax);
Z(Zmode);
Z(Zmrkpara);
Z(Znewline);
Z(Znextbuff);
Z(Znextchar);
Z(Znexterr);
Z(Znextline);
Z(Znextpage);
Z(Znextwind);
Z(Znotimpl);
Z(Znxtbookmrk);
Z(Znxtothrwind);
Z(Zopenline);
Z(Zoverin);
Z(Zpart);
Z(Zprevchar);
Z(Zprevline);
Z(Zprevothrwind);
Z(Zprevpage);
Z(Zprevwind);
Z(Zprintpos);
Z(Zquery);
Z(Zquote);
Z(Zrcsci);
Z(Zrcsco);
Z(Zrdelchar);
Z(Zrdelword);
Z(Zredisplay);
Z(Zref);
Z(Zreplace);
Z(Zrereplace);
Z(Zresrch);
Z(Zgresrch);
Z(Zrincsrch);
Z(Zrsearch);
Z(Zsaveall);
Z(Zsavebind);
Z(Zshowconfig);
Z(Zscrolldown);
Z(Zscrollup);
Z(Zsearch);
Z(Zsetavar);
Z(Zsetbookmrk);
Z(Zsetenv);
Z(Zsetmrk);
Z(Zshell);
Z(Zshrinkwind);
Z(Zsizewind);
Z(Zspell);
Z(Zswapchar);
Z(Zswapmrk);
Z(Zswapword);
Z(Zswitchto);
Z(Ztab);
Z(Ztoend);
Z(Ztostart);
Z(Zundent);
Z(Zundo);
Z(Zunmodf);
Z(Zupregion);
Z(Zupword);
Z(Zviewline);
Z(Zvmode);
Z(Zyank);

Z(pinsert);
Z(pnewline);

/* General routines */

#ifdef BSD
int access(char *, int);
#endif
void initscrnmarks(void);
int ask(char *);
int ask2(char *, Boolean);
void free_extensions(void);
void bfini(void);
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
Boolean bindfile(char *fname, int mode);
void binsert(Byte);
void binstr(char *);
Boolean bisaftermrk(struct mark *);
Boolean bisbeforemrk(struct mark *);
long blength(struct buff *);
long blines(struct buff *);
unsigned long blocation(unsigned *);
int bmakecol(int, Boolean);
void bmrktopnt(struct mark *);
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
Boolean delay(int ms);
Boolean delayprompt(char *);
Boolean delbname(char *);
void delbuff(struct buff *);
Boolean delcmd(void);
Boolean delcmdall(void);
char *dispkey(unsigned, char *);
void execute(void);
void extendedlinemarker(void);
Boolean filesave(void);
Boolean findfile(char *);
int findpath(char *, char *);
struct wdo *findwdo(struct buff *);
Boolean getarg(char *, char *, int);
char *getbtxt(char *, int);
Boolean getbword(char *, int, int (*)());
int getdname(char *prompt, char *path);
int getplete(char *, char *, char **, int, int);
int bisspace(void);
int bistoken(void);
int biswhite(void);
int bisword(void);
void killtomrk(struct mark *);
char *lastpart(char *);
char *limit(char *, int);
void makecur(struct page *);
void makeoffset(int);
void makepaw(char *, Boolean);
void movepast(int (*pred)(), Boolean forward);
void moveto(int (*pred)(), Boolean forward);
Boolean mrkaftermrk(struct mark *, struct mark *);
Boolean mrkatmrk(struct mark *, struct mark *);
Boolean mrkbeforemrk(struct mark *, struct mark *);
char *nocase(char *);
void parsem(char *, Boolean);
int pathfixup(char *, char *);
void pntmove(int, int);
void pout(char *, Boolean);
void pset(int, int, int);
int prefline(void);
void putpaw(const char *fmt, ...);
void readvfile(void);
void redisplay(void);
void reframe(void);
void zrefresh(void);
void regerr(int);
Boolean paw_resize(int);
void save(struct buff *);
Boolean saveall(Boolean);
int settabsize(unsigned);
Boolean step(Byte *);
void setmark(Boolean);
char *strup(char *);
void tbell(void);
void tcleol(void);
void tclrwind(void);
void termsize(void);
void tfini(void);
void tforce(void);
int tgetcmd(void);
Byte tgetkb(void);
void tpushcmd(int cmd);
void t_goto(int, int);
void tindent(int);
void tinit(void);
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
void vfini(void);
int chwidth(Byte, int, Boolean);
void hangup(int);
Boolean notdup_key(int k);

/* Terminal driver specific routines */
void tlinit(void);
void tlfini(void);

/* compile switched routines */
void message(struct buff *, char *);

int dopipe(struct buff *, char *);
void winit(void);
void wfini(void);

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

/* for getfname */

int getfname(char *, char *);
int nmatch(char *, char *);


void wswitchto(struct wdo *wdo);
void wsize(void);
Boolean wuseother(char *);

#if COMMENTBOLD
void addcomment(void);
#define addcpp addcomment
void resetcomments(void);
void checkcomment(void);
void recomment(void);
void uncomment(struct buff *buff, int need_update);
#else
static inline void addcomment(void) {}
static inline void resetcomments(void) {}
static inline void checkcomment(void) {}
static inline void recomment(void) {}
static inline void uncomment(struct buff *buff, int need_update) {}
#endif

#if UNDO
void undo_add(int size);
void undo_del(int size);
#else
static inline void undo_add(int size) {}
static inline void undo_del(int size) {}
#endif
void undo_clear(struct buff *buff);
void ufini(void);
