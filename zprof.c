/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include <stdio.h>
#include <pwd.h>
#include "global.h"


/*
 * This program produces formatted output from a cmd count file produced by
 * Zedit with the CMDPROF define set. The usage is: zprof [-v] [-s] user.
 * If -v is set, output is given for all commands. The default is to only
 * display non-zero commands. If -s is set, the command and ascii will
 * be sorted in order of use.
 */
 
struct cnames
{
	char *name;
	Word fnum;
	Word htype;
};

/* Help Types */
#define H_NONE				0
#define H_MISC				1
#define H_VAR				2
#define H_CURSOR			3
#define H_DELETE			4
#define H_SEARCH			5
#define H_MACRO				6
#define H_FILE				7
#define H_BUFF				8
#define H_DISP				9
#define H_MODE				10
#define H_HELP				11
#define H_BIND				12
#define H_SHELL				13

#include "funcs.h"
#include "cnames.h"

#define NUMPRINT	('~' - ' ' + 1)
unsigned long cmdcnt[NUMFUNCS + NUMPRINT];

struct cnt {
	union {
		char u_ch;
		char *u_name;
	} str;
#define e_ch	str.u_ch
#define e_name str.u_name
	int cnt;
} cmd[NUMFUNCS], ascii[NUMPRINT];


/* compare function for qsort */
int comp(i, j)
struct cnt *i, *j;
{
	return(j->cnt - i->cnt);
}


void main(argc, argv)
int argc;
char *argv[];
{
	extern char *getenv();
	extern int optind, errno;
	int verbose = 0, sorted = 1, errflag = 0;
	int fd;
	int i, j;
	unsigned long charcnt = 0;
	char fname[256], *uname;
	struct passwd *user;
	unsigned long total_cnt = 0;

	while((i = getopt(argc, argv, "vs")) != -1)
		switch(i) {
			case 'v':	verbose = 1;	break;
			case 's':	sorted = 0;		break;
			case '?':	errflag = 1;	break;
		}

	if(errflag)
	{
		fputs("usage: zprof [-v] [-s] [user]\n", stdout);
		return;
	}
	
	if(optind < argc)
		uname = argv[optind];
	else if((uname = getenv("LOGNAME")) == NULL)
	{
		printf("LOGNAME not set\n");
		exit(1);
	}
	if((user = getpwnam(uname)) == 0)
	{
		fprintf(stderr, "Unable to find user %s\n", argv[optind]);
		exit(1);
	}
	sprintf(fname, "%s/.cmdcnt.z", user->pw_dir);
	
	if((fd = open(fname, 0)) != -1)
	{
		if((i = read(fd, (char *)cmdcnt, sizeof(cmdcnt))) == sizeof(cmdcnt))
		{
			for(i = 0; i < NUMFUNCS; ++i)
			{
				for(j = 0; j < NUMFUNCS && Cnames[j].fnum != i; ++j) ;
				if(j == NUMFUNCS)
				{
					fprintf(stderr, "Unable to match %d\n", i);
					cmd[i].e_name = "???";
				}
				else
					cmd[i].e_name = Cnames[j].name;
				cmd[i].cnt = cmdcnt[i];
				total_cnt += cmdcnt[i];
			}
			for(j = 0; j < NUMPRINT; ++j, ++i)
			{
				ascii[j].e_ch = j + ' ';
				ascii[j].cnt = cmdcnt[i];
			}

			if(sorted)
			{
				qsort(cmd,  NUMFUNCS, sizeof(struct cnt), comp);
				qsort(ascii, NUMPRINT, sizeof(struct cnt), comp);
			}

			printf("%sCOMMANDS\n", sorted ? "SORTED " : "");
			for(i = 0; i < NUMFUNCS; ++i)
				if(cmd[i].cnt || verbose)
					printf("%-25s  %d (%u%%)\n",
						cmd[i].e_name, cmd[i].cnt,
						cmd[i].cnt * 100 / total_cnt);
			printf("Command count: %u\n", total_cnt);

			printf("\n%sASCII CHARACTER SET\n\n", sorted ? "SORTED " : "");
			for(i = 0; i < NUMPRINT; )
			{
				printf("%c  %5d", ascii[i].e_ch, ascii[i].cnt);
				putchar((++i & 3) ? '\t' : '\n');
				charcnt += ascii[i].cnt;
			}
			putchar('\n');
			printf("Character count: %u\n", charcnt);
		}
		else if(i == -1)
			perror("unable to read .cmdcnt.z");
		else
			fputs("wrong version!\n", stderr);
		close(fd);
	}
	else perror(fname);
}
