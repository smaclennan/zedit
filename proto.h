/* Copyright (C) 1988-2018 Sean MacLennan */

/** @addtogroup zedit
 * @{
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
Z(Zlife);
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
Z(Zuntab);
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

int ask(const char *str);
int bgetcol(bool flag, int col);
void binit(void);
int bmakecol(int col);
void zswitchto(struct zbuff *buff);
struct zbuff *cfindbuff(const char *bname);
struct zbuff *cfindzbuff(struct buff *buff);
struct zbuff *zcreatebuff(const char *bname, char *fname);
struct zbuff *cmakebuff(const char *bname, char *fname);
bool cdelbuff(struct zbuff *buff);
void cswitchto(struct zbuff *buff);
void display_init(struct mark *mrk);
int delayprompt(const char *prompt);
void delinit(void);
void delfini(void);
unsigned int delpages(void);
bool delcmd(void);
int do_chdir(struct zbuff *buff);
void execute(void);
int readapipe(void);
void set_pipefd(int fd);
bool filesave(void);
bool findfile(char *fname);
struct wdo *findwdo(struct buff *buff);
bool getarg(const char *prompt, char *arg, int max);
bool _getarg(const char *prompt, char *arg, int max, bool tostart);
char *getbtxt(char *text, int len);
int getbword(char *word, int len, int (*func)(int));
int getplete(const char *prompt, const char *def, char **array,
	     int size, int num);
int bistoken(int c);
int biswhite(int c);
int bisword(int c);
int file_mode(void);
void killtomrk(struct mark *mark);
char *lastpart(char *fname);
char *limit(char *fname, int num);
void makepaw(char *str, bool start);
char *nocase(const char *str);
int pathfixup(char *to, char *from);
bool promptsave(struct zbuff *tbuff, bool must);
int prefline(void);
void putpaw(const char *fmt, ...);
void error(const char *fmt, ...);
void readvfile(const char *path);
void redisplay(void);
void reframe(void);
void zrefresh(void);
bool saveall(bool must);
int settabsize(unsigned int size);
void tbell(void);
void tainit(void);
void termsize(void);
void tindent(int arg);
void toggle_mode(int mode);
void tprntchar(Byte ch);
void tprntstr(const char *str);
void vsetmrk(struct mark *mark);
void invalidate_scrnmarks(unsigned int from, unsigned int to);
int chwidth(Byte ch, int col, bool adjust);
void hang_up(int signo);
void dump_doc(const char *doc);
int zreadfile(char *fname);
void set_shell_mark(void);
void message(struct zbuff *buff, const char *str);

int batoi(void);

void vsetmod_callback(struct buff *buff);
#define vsetmod() vsetmod_callback(NULL)
void set_sstart(struct mark *mrk);

/* umark routines */
int _set_umark(struct mark *tmark); /* tmark == NULL means set to point */
void set_umark(struct mark *tmark); /* tmark == NULL means set to point */
void clear_umark(void);

void winit(void);

void checkpipes(int type);
void siginit(void);
bool unvoke(struct zbuff *buff);

/* for getfname */
int getfname(const char *prompt, char *path);
int nmatch(char *s1, char *s2);

void wswitchto(struct wdo *wdo);
void wsize(void);
bool wuseother(const char *bname);
void wgoto(struct buff *buff);

/* COMMENTBOLD */
void resetcomments(void);
void uncomment(struct zbuff *buff);
void cprntchar(Byte ch);
/* @} */
