/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#if SUNBSD

/* since sun include files not ansi style unless using sys5 mode */

/* extremely low file IO */
int _flsbuf ARGS((unsigned char, FILE*));
int _filbuf ARGS((FILE *));

/* low level file IO */
int close ARGS((int));
int read ARGS((int, char*, int));
int write ARGS((int, char*, int));
int unlink ARGS((char*));
int link ARGS((char*, char*));
int ioctl ARGS((int, int, ...));
int lseek ARGS((int, long, int));

/* high level file IO */
char *fputs ARGS((char *, FILE *));
int fputc ARGS((char, FILE*));
int fread  ARGS((char*, int, int, FILE*));
int fwrite ARGS((char*, int, int, FILE*));
int fprintf ARGS((FILE *, char*, ...));
int fscanf ARGS((FILE *, char*, ...));
int rewind ARGS((FILE *));
int fflush ARGS((FILE *));
int fclose ARGS((FILE *));

int vprintf ARGS((char *, ...));
int vfprintf ARGS((FILE *, char *, ...));

int printf ARGS((char*, ...));
char *puts ARGS((char*));

/* memory alloc */
char *malloc ARGS((int));
int free ARGS((char *));

/* pipe stuff */
int wait ARGS((int*));
int pipe ARGS((int fd[2]));
int fork ARGS((void));
int dup2 ARGS((int, int));
int execvp ARGS((char *, char**));

/* misc */
int select ARGS((int, int *, int *, int *, void *));
int system ARGS((char*));
void abort ARGS((void));
long time ARGS((long*));
int getpid ARGS((void));
int geteuid ARGS((void));
char *mktemp ARGS((char*));
int pause ARGS((void));
#ifndef __GNUC__
int exit ARGS((int));
#endif
long strtol ARGS((char*, char**, int));
int abs ARGS((int));
int atoi ARGS((char*));
int getopt ARGS((int, char **, char *));

#ifndef SUN3
char tolower ARGS((char));
char toupper ARGS((char));
#endif

#endif
