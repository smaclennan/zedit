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
Z(Zone_window);
Z(Zsplit_window);
Z(Zabort);
Z(Zagain);
Z(Zarg);
Z(Zbeginning_of_line);
Z(Zprevious_paragraph);
Z(Zprevious_word);
Z(Zcalc);
Z(Zcapitalize_word);
Z(Ztoggle_case);
Z(Zcenter);
Z(Zout_to);
Z(Zc_indent);
Z(Zc_insert);
Z(Zcmd_to_buffer);
Z(Zcopy_region);
Z(Zcount);
Z(Zctrl_x);
Z(Zdelete_blanks);
Z(Zdelete_char);
Z(Zdelete_to_eol);
Z(Zdelete_line);
Z(Zdelete_region);
Z(Ztrim_white_space);
Z(Zdelete_word);
Z(Zrevert_file);
Z(Zempty_buffer);
Z(Zend_of_line);
Z(Zexit);
Z(Zsave_and_exit);
Z(Zread_file);
Z(Zsave_file);
Z(Zwrite_file);
Z(Zfill_check);
Z(Zfill_paragraph);
Z(Zfind_file);
Z(Zfname);
Z(Znext_paragraph);
Z(Znext_word);
Z(Zcopy_word);
Z(Zgrep);
Z(Zglobal_search);
Z(Zgrow_window);
Z(Zincremental_search);
Z(Zindent);
Z(Zinsert);
Z(Zjoin);
Z(Zkill);
Z(Zdelete_buffer);
Z(Zgoto_line);
Z(Zlowercase_region);
Z(Zlowercase_word);
Z(Zlist_buffers);
Z(Zmake);
Z(Zappend_kill);
Z(Zmeta);
Z(Zmeta_x);
Z(Zmode);
Z(Zmark_paragraph);
Z(Znewline);
Z(Znext_buffer);
Z(Znext_char);
Z(Znext_error);
Z(Znext_line);
Z(Znext_page);
Z(Znext_window);
Z(Znotimpl);
Z(Znext_bookmark);
Z(Zother_next_page);
Z(Zopen_line);
Z(Zinsert_overwrite);
Z(Zpart);
Z(Zprevious_char);
Z(Zprevious_line);
Z(Zother_previous_page);
Z(Zprevious_page);
Z(Zposition);
Z(Zquery_replace);
Z(Zquote);
Z(Zdelete_previous_char);
Z(Zdelete_previous_word);
Z(Zredisplay);
Z(Zreplace);
Z(Zre_replace);
Z(Zre_search);
Z(Zglobal_re_search);
Z(Zreverse_search);
Z(Zsave_all_files);
Z(Zshow_config);
Z(Zscroll_down);
Z(Zscroll_up);
Z(Zsearch);
Z(Zset_variable);
Z(Zset_bookmark);
Z(Zsetenv);
Z(Zset_mark);
Z(Zsize_window);
Z(Zswap_chars);
Z(Zswap_mark);
Z(Zswap_words);
Z(Zswitch_to_buffer);
Z(Ztab);
Z(Zend_of_buffer);
Z(Zbeginning_of_buffer);
Z(Zundent);
Z(Zundo);
Z(Zunmodify);
Z(Zuppercase_region);
Z(Zuppercase_word);
Z(Zyank);
Z(Zhelp);
Z(Zhelp_apropos);
Z(Zhelp_function);
Z(Zhelp_key);
Z(Zhelp_variable);
Z(Zstats);
Z(Zword_search);
Z(Zspell_word);
Z(Ztag);
Z(Ztag_word);
Z(Zversion);
Z(Zzap_to_char);

Z(pinsert);
Z(pnewline);

/* General routines */

int ask(const char *);
int ask2(const char *, bool);
int bgetcol(bool, int);
int bmakecol(int, bool);
struct buff *cfindbuff(const char *);
struct buff *cmakebuff(const char *, char *);
int compile(Byte*, Byte*, Byte*);
void cswitchto(struct buff *);
void display_init(void);
bool tdelay(int ms);
int delayprompt(const char *);
void delinit(void);
void delfini(void);
void delbuff(struct buff *);
bool delcmd(void);
int do_chdir(struct buff *buff);
void execute(void);
int readapipe(struct buff *tbuff);
bool fd_add(int fd, struct buff *buff);
void fd_remove(int fd);
bool filesave(void);
bool findfile(char *);
struct wdo *findwdo(struct buff *);
bool getarg(const char *, char *, int);
bool _getarg(const char *, char *, int, bool);
char *getbtxt(char *, int);
int getbword(char *, int, int (*)());
bool looking_at(const char *match);
int getplete(const char *, const char *, char **, int, int);
int bisspace(void);
int bistoken(void);
int biswhite(void);
int bisword(void);
void killtomrk(struct mark *);
char *lastpart(char *);
char *limit(char *, int);
void makepaw(char *, bool);
void movepast(int (*pred)(), bool forward);
void moveto(int (*pred)(), bool forward);
bool mrkaftermrk(struct mark *, struct mark *);
bool mrkatmrk(struct mark *, struct mark *);
bool mrkbeforemrk(struct mark *, struct mark *);
char *nocase(const char *);
int pathfixup(char *, char *);
bool promptsave(struct buff *tbuff, bool must);
int prefline(void);
void _putpaw(const char *str);
void putpaw(const char *fmt, ...);
void error(const char *fmt, ...);
void readvfile(const char *path);
void redisplay(void);
void reframe(void);
void zrefresh(void);
void regerr(int);
bool saveall(bool);
int settabsize(unsigned);
bool step(Byte *);
void setmark(bool);
void tbell(void);
void tcleol(void);
void tclrwind(void);
void termsize(void);
void tfini(void);
void tforce(void);
void tsetcursor(bool hide);
int tgetcmd(void);
void t_goto(int, int);
void tindent(int);
void tinit(void);
bool tkbrdy(void);
void tobegline(void);
void toendline(void);
void toggle_mode(int);
void tprntchar(Byte);
void tprntstr(const char *);
void tstyle(int);
void vsetmrk(struct mark *);
int chwidth(Byte, int, bool);
void hang_up(int);
void dump_doc(const char *doc);
int set_bookmark(char *bookname);
void cleanup_bookmarks(void);
const char *special_label(int key);
struct mark *zcreatemrk(void);
bool zwritefile(char *fname);

#if GPM_MOUSE
void handle_gpm_mouse(void);
void handle_mouse_cursor(void);
#endif

/* umark routines */
void set_umark(struct mark *tmark); /* tmark == NULL means set to point */
void clear_umark(void);

void winit(void);
void wfini(void);

void checkpipes(int type);
void siginit(void);
void unvoke(struct buff *);

void Dbg(const char *fmt, ...);
const char *func2name(Byte func);

/* for getfname */
int getfname(const char *, char *);
int nmatch(char *, char *);

void wswitchto(struct wdo *wdo);
void wsize(void);
bool wuseother(const char *);
struct wdo *wfind(int row);

void set_mouse(bool enable);
void mouse_scroll(int row, bool down);
void mouse_point(int row, int col, bool set_mark);

/* COMMENTBOLD */
void resetcomments(void);
void uncomment(struct buff *buff);
void cprntchar(Byte ch);

#if UNDO
extern unsigned long undo_total;
void undo_add(int size);
void undo_del(int size);
void undo_clear(struct buff *buff);
#else
#define undo_add(size)
#define undo_del(size)
#define undo_clear(buff)
#endif
