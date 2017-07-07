/* proto.h - Zedit function prototypes
 * Copyright (C) 1988-2016 Sean MacLennan
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
Z(Zreverse_search);
Z(Zsave_all_files);
Z(Zshow_config);
Z(Zsearch);
Z(Zset_variable);
Z(Zset_bookmark);
Z(Zsetenv);
Z(Zset_mark);
Z(Zsh_indent);
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
Z(Zzap_to_char);
Z(Zdos2unix);

Z(pinsert);
Z(pnewline);

/* General routines */

int ask(const char *);
int bgetcol(bool, int);
int bmakecol(int);
void zswitchto(struct zbuff *);
struct zbuff *cfindbuff(const char *);
struct zbuff *cfindzbuff(struct buff *buff);
struct zbuff *cmakebuff(const char *, char *);
bool cdelbuff(struct zbuff *buff);
void cswitchto(struct zbuff *);
void display_init(struct mark *mrk);
int delayprompt(const char *);
void delinit(void);
void delfini(void);
unsigned delpages(void);
bool delcmd(void);
int do_chdir(struct zbuff *buff);
void execute(void);
int readapipe(void);
bool fd_add(int fd);
void fd_remove(int fd);
bool filesave(void);
bool findfile(char *);
struct wdo *findwdo(struct buff *);
bool getarg(const char *, char *, int);
bool _getarg(const char *, char *, int, bool);
char *getbtxt(char *, int);
int getbword(char *, int, int (*)(int));
int getplete(const char *, const char *, char **, int, int);
int bistoken(int c);
int biswhite(int c);
int bisword(int c);
int file_mode(void);
void killtomrk(struct mark *);
char *lastpart(char *);
char *limit(char *, int);
void makepaw(char *, bool);
char *nocase(const char *);
int pathfixup(char *, char *);
bool promptsave(struct zbuff *tbuff, bool must);
int prefline(void);
void putpaw(const char *fmt, ...);
void error(const char *fmt, ...);
void readvfile(const char *path);
void redisplay(void);
void reframe(void);
void zrefresh(void);
bool saveall(bool);
int settabsize(unsigned);
void setmark(bool);
void shell_init(void);
void tbell(void);
void tainit(void);
void termsize(void);
void tsetcursor(void);
void tindent(int);
void toggle_mode(int);
void tprntchar(Byte);
void tprntstr(const char *);
void vsetmrk(struct mark *);
void invalidate_scrnmarks(unsigned from, unsigned to);
int chwidth(Byte, int, bool);
void hang_up(int);
void dump_doc(const char *doc);
int zreadfile(char *fname);
void set_shell_mark(void);
void message(struct zbuff *buff, const char *str);

int batoi(void);

void vsetmod_callback(struct buff *buff);
#define vsetmod() vsetmod_callback(NULL);
void set_sstart(struct mark *mrk);

/* umark routines */
void set_umark(struct mark *tmark); /* tmark == NULL means set to point */
void clear_umark(void);

void winit(void);

void checkpipes(int type);
void siginit(void);
bool unvoke(struct zbuff *);

/* for getfname */
int getfname(const char *, char *);
int nmatch(char *, char *);

void wswitchto(struct wdo *wdo);
void wsize(void);
bool wuseother(const char *);
struct wdo *wfind(int row);
void wgoto(struct buff *);

/* COMMENTBOLD */
void resetcomments(void);
void uncomment(struct zbuff *buff);
void cprntchar(Byte ch);
