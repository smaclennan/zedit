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
Z(Zbeginning_of_line);
Z(Zbegwind);
Z(Zbind);
Z(Zbpara);
Z(Zprevious_word);
Z(Zcalc);
Z(Zcapitalize_word);
Z(Zcase);
Z(Zcenter);
Z(Zcexpand);
Z(Zcfindexpand);
Z(Zout_to);
Z(Zc_indent);
Z(Zc_insert);
Z(Zcmd_to_screen);
Z(Zcmd_to_buffer);
Z(Zbound_to);
Z(Zcmode);
Z(Zcopy_region);
Z(Zcount);
Z(Zctrl_x);
Z(Zcwd);
Z(Zdelete_blanks);
Z(Zdelete_char);
Z(Zdelete_to_eol);
Z(Zdelete_line);
Z(Zdelete_region);
Z(Zdelwhite);
Z(Zdelwind);
Z(Zdelete_word);
Z(Zdispbinds);
Z(Zrevertfile);
Z(Zempty_buffer);
Z(Zend_of_line);
Z(Zendwind);
Z(Zexit);
Z(Zsave_and_exit);
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
Z(Znext_word);
Z(Zcopy_word);
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
Z(Zlife);
Z(Zgoto_line);
Z(Zlowregion);
Z(Zlowercase_word);
Z(Zlstbuff);
Z(Zmake);
Z(Zappend_kill);
Z(Zmeta);
Z(Zmeta_x);
Z(Zmode);
Z(Zmrkpara);
Z(Znewline);
Z(Znextbuff);
Z(Znext_char);
Z(Znexterr);
Z(Znext_line);
Z(Znext_page);
Z(Znextwind);
Z(Znotimpl);
Z(Znext_bookmark);
Z(Znxtothrwind);
Z(Zopen_line);
Z(Zoverin);
Z(Zpart);
Z(Zprevious_char);
Z(Zprevious_line);
Z(Zprevothrwind);
Z(Zprevious_page);
Z(Zprevwind);
Z(Zprintpos);
Z(Zquery);
Z(Zquote);
Z(Zdelete_previous_char);
Z(Zdelete_previous_word);
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
Z(Zscroll_down);
Z(Zscroll_up);
Z(Zsearch);
Z(Zsetavar);
Z(Zset_bookmark);
Z(Zsetenv);
Z(Zsetmrk);
Z(Zshell);
Z(Zshrinkwind);
Z(Zsizewind);
Z(Zspell);
Z(Zswap_chars);
Z(Zswap_mark);
Z(Zswap_words);
Z(Zswitchto);
Z(Ztab);
Z(Zend_of_buffer);
Z(Zbeginning_of_buffer);
Z(Zundent);
Z(Zundo);
Z(Zunmodf);
Z(Zupregion);
Z(Zuppercase_word);
Z(Zview_line);
Z(Zvmode);
Z(Zyank);

Z(pinsert);
Z(pnewline);

/* General routines */

void initscrnmarks(void);
int ask(char *);
int ask2(char *, bool);
void free_extensions(void);
void bfini(void);
int bcopyrgn(struct mark *, struct buff*);
struct buff *bcreate(void);
struct mark *bcremrk(void);
bool bcrsearch(Byte);
bool bcsearch(Byte);
bool bdelbuff(struct buff *);
void bdelete(unsigned);
void bdeltomrk(struct mark *);
void bempty(void);
int bgetcol(bool, int);
void bgoto(struct buff *);
void bind(void);
bool bindfile(char *fname, int mode);
void binsert(Byte);
void binstr(char *);
bool bisaftermrk(struct mark *);
bool bisbeforemrk(struct mark *);
long blength(struct buff *);
long blines(struct buff *);
unsigned long blocation(unsigned *);
int bmakecol(int, bool);
void bmrktopnt(struct mark *);
void bpnttomrk(struct mark *);
int breadfile(char *);
bool bstrsearch(char *, bool);
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
bool delay(int ms);
bool delayprompt(char *);
bool delbname(char *);
void delbuff(struct buff *);
bool delcmd(void);
char *dispkey(unsigned, char *);
void execute(void);
bool filesave(void);
bool findfile(char *);
int findpath(char *, char *);
struct wdo *findwdo(struct buff *);
bool getarg(char *, char *, int);
char *getbtxt(char *, int);
bool getbword(char *, int, int (*)());
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
void makepaw(char *, bool);
void movepast(int (*pred)(), bool forward);
void moveto(int (*pred)(), bool forward);
bool mrkaftermrk(struct mark *, struct mark *);
bool mrkatmrk(struct mark *, struct mark *);
bool mrkbeforemrk(struct mark *, struct mark *);
char *nocase(char *);
void parsem(char *, int);
int pathfixup(char *, char *);
bool promptsave(struct buff *tbuff, bool must);
void pntmove(int, int);
void pout(char *, bool);
void pset(int, int, int);
int prefline(void);
void putpaw(const char *fmt, ...);
void readvfile(void);
void redisplay(void);
void reframe(void);
void zrefresh(void);
void regerr(int);
bool paw_resize(int);
void save(struct buff *);
bool saveall(bool);
int settabsize(unsigned);
bool step(Byte *);
void setmark(bool);
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
void vsetmod(bool);
void vsetmrk(struct mark *);
void vfini(void);
int chwidth(Byte, int, bool);
void hang_up(int);
bool notdup_key(int k);

/* Terminal driver specific routines */
void tlinit(void);
void tlfini(void);

/* compile switched routines */
void message(struct buff *, char *);

bool dopipe(struct buff *, char *);
void winit(void);
void wfini(void);

#ifdef PIPESH
int checkpipes(int);
bool doshell(void);
bool invoke(struct buff *, char **);
int readpipes(fd_set *);
void sendtopipe(void);
void sigchild(int);
#endif

void unvoke(struct buff *, bool);

void Dbg(char *fmt, ...);

/* for getfname */

int getfname(char *, char *);
int nmatch(char *, char *);


void wswitchto(struct wdo *wdo);
void wsize(void);
bool wuseother(char *);

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
void undo_clear(struct buff *buff);
void ufini(void);
#else
static inline void undo_add(int size) {}
static inline void undo_del(int size) {}
static inline void undo_clear(struct buff *buff) {}
static inline void ufini(void) {}
#endif
