#include "../z.h"

#if XWINDOWS
#include <sys/socket.h>

static void addSocket ARGS((int sock));
static void AcceptSocket ARGS((void));

/************************************************************************/
/* The following code handles sockets/xevents.							*/
/************************************************************************/

extern fd_set SelectFDs;
extern int NumFDs;
int Xfd;

/* we must save this since we create the .zb file before we spawn */
static pid_t MyPid;

#define MAXACCEPT	8
static int MySock = -1;
static int Pipe = -1;
static struct sockaddr outSock;
static int AcceptSock[MAXACCEPT];
static char *Name[MAXACCEPT];
static char fname[16];

static void WriteSocket(int socket, char *str);
static void CheckSockets(fd_set *readfds);
static void ListAllVars(int fd);
static void SendVariable(int fd, int i);

/* Called by xinit to initialize the checkfds with the Xfd fd.
 * Also create the socket for other programs to connect to.
 * It is fatal if we cannot create socket.
 */
void initSockets(xfd)
int xfd;
{
	int i;

	MyPid = getpid();

	/* fname must be < 14 */
	sprintf(fname, "/tmp/.zbl%04x", MyPid);
	unlink(fname);

	if((MySock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(2);
	}

	outSock.sa_family = AF_UNIX;
	strcpy(outSock.sa_data, fname);
	if(bind(MySock, &outSock, sizeof(outSock)) == -1)
	{
		perror("bind");
		exit(2);
	}
	if(listen(MySock, 5) == -1)
	{
		perror("listen");
		exit(2);
	}
	
	for(i = 0; i < MAXACCEPT; ++i)
	{
		AcceptSock[i] = -1;
		Name[i] = 0;
	}
		
	
	Xfd = xfd;

	FD_ZERO(&SelectFDs);
	addSocket(xfd);
	addSocket(MySock);
}

/* Called by Quit */
void closeSockets()
{
	CleanupSocket(-1);
	close(MySock);
}

/* Process socket calls. Return only if we get an Xevent. */
void ProcessFDs()
{
	fd_set readfds, exceptFDs;

	while(1)
	{
		readfds = SelectFDs;
		if(select(NumFDs, &readfds, 0, 0, 0) <= 0)
			continue;
		if(FD_ISSET(MySock, &readfds))
			AcceptSocket();
#ifdef PIPESH
		if(Readpipes(&readfds))
		{
			ShowCursor(FALSE);
			Refresh();
			ShowCursor(TRUE);
			Xflush();
		}
#endif
		CheckSockets(&readfds);
		if(FD_ISSET(Xfd, &readfds))
			return;
	}
}

/* Pipe is not really a socket - but who cares? */
void addPipe(pipe)
int pipe;
{
	if(Pipe != -1)
	{
		Tbell();
		return;
	}
	addSocket(Pipe = pipe);
}

/* Add a socket to the checkfds */
static void addSocket(sock)
int sock;
{
	FD_SET(sock, &SelectFDs);
	/* SAM NumFDs only rises, we never lower it if a socket is removed */
	if(sock >= NumFDs) NumFDs = sock + 1;
}

/* Remove a socket from the checkfds */
void removeSocket(sock)
int sock;
{
	if(sock == Pipe) Pipe = -1;
	FD_CLR(sock, &SelectFDs);
}

/* Someone wants to connect to us! */
static void AcceptSocket()
{
	int i;
	int len = sizeof(outSock);

	/* find an empty slot */
	for(i = 0; i < MAXACCEPT && AcceptSock[i] != -1; ++i) ;
	if(i == MAXACCEPT) return;

	if((AcceptSock[i] = accept(MySock, &outSock, &len)) != -1)
		addSocket(AcceptSock[i]);
}


/************************************************************************/
/* The following code is for the "popup" programs						*/
/************************************************************************/
	
int StartProg(prog)
char *prog;
{
	pid_t child;
	
	if((child = fork()) == 0)
	{
		char str[10];
		sprintf(str, "%d", MyPid);
		execlp(prog, prog, "-pid", str, 0);
		perror("execlp");
		exit(1);
	}

	if(child == 0) return errno;

	return 0;
}


static int BlistSocket = -1;

static void CheckSockets(readfds)
fd_set *readfds;
{
	extern char Fname[];
	int i;

	for(i = 0; i < MAXACCEPT; ++i)
	{
		if(AcceptSock[i] != -1 && FD_ISSET(AcceptSock[i], readfds))
		{
			extern Buffer *Bufflist;
			Buffer *buff;
			char cmd[1024], *p;
			int n;

			if((n = read(AcceptSock[i], cmd, sizeof(cmd))) > 0)
			{
				switch(*cmd)
				{
					case 'I':		/* init message */
						Name[i] = strdup(cmd + 1);
						sprintf(cmd, "Started %s.", Name[i]);
						Echo(cmd);
						if(strcmp(Name[i], "Zblist") == 0)
						{	/* list all buffers */
							BlistSocket = AcceptSock[i];
							for(buff = Bufflist; buff; buff = buff->next)
								if((buff->bmode & SYSBUFF) == 0)
									XAddBuffer(buff->bname);
							/* highlight current buffer */
							XSwitchto(Curbuff->bname);
						}
						else if(strcmp(Name[i], "Zmake") == 0)
							RunMakeCmd();
						else if(strcmp(Name[i], "Zvarhelp") == 0)
							ListAllVars(AcceptSock[i]);
						break;
	
					case 'B':		/* Zswitchto */
						if(InPaw) { Tbell(); break; }
						if((buff = Cfindbuff(cmd + 1)) != 0 &&
							buff != Curbuff)
						{
							extern char Lbufname[];

							strcpy(Lbufname, Curbuff->bname);
							Loadwdo(cmd + 1);
							ShowCursor(FALSE);
							Cswitchto(buff);
							Refresh();
							ShowCursor(TRUE);
							Xflush();
						}
						break;
						
					case 'F':		/* Zfindfile */
						if(InPaw) { Tbell(); break; }
						Pathfixup(Fname, &cmd[1]);
						Findfile(Fname, FALSE);
						Refresh();
						Xflush();
						break;
					
					case 'M':		/* Make */
						if(*(cmd + 1))
						{	/* update make command */
							extern char mkcmd[];
							strcpy(mkcmd, cmd + 1);
						}
						Argp = FALSE;
						Zmake();
						break;

					case 'N':		/* Nexterr */
						n = strtol(cmd + 1, &p, 10);
						if(*p == ':')
						{
							char fname[PATHMAX + 1];
							Pathfixup(fname, p + 1);
							Findfile(fname, FALSE);						
							Argp = TRUE;
							Arg = n;
							Zlgoto();
							Tobegline();
							ShowCursor(FALSE);
							Refresh();
							ShowCursor(TRUE);
							Xflush();
						}
						break;
						
					case 'V':		/* set a variable */
						if(strcmp(cmd, "Vsave") == 0)
						{	/* Save file in current directory. */
							Argp = TRUE;
							Zsaveconfig();
							Argp = FALSE;
						}
						else if((n = XSetAVar(cmd + 1)) != -1 && n < VARNUM)
							SendVariable(AcceptSock[i], n);
						break;
				}
			}
			else
				CleanupSocket(i);
		}
	}
}


void CleanupSocket(i)
int i;
{
	if(i == -1)
	{	/* close all sockets */
		for(i = 0; i < MAXACCEPT; ++i)
		{
			if(AcceptSock[i] != -1)
			{
				removeSocket(AcceptSock[i]);
				close(AcceptSock[i]);
				AcceptSock[i] = -1;
				free(Name[i]);
				Name[i] = 0;
			}
		}

		unlink(fname);
		BlistSocket = -1;
	}
	else
	{	/* close specified socket */
		if(AcceptSock[i] == BlistSocket) BlistSocket = -1;
		if(AcceptSock[i] != -1)
		{
			removeSocket(AcceptSock[i]);
			close(AcceptSock[i]);
			AcceptSock[i] = -1;
			free(Name[i]);
			Name[i] = 0;
		}
	}
}


void XAddBuffer(char *bname)
{
	char cmd[BUFNAMMAX + 2];
	
	if(BlistSocket != -1)
	{
		*cmd = 'A';
		strcpy(cmd + 1, bname);
		WriteSocket(BlistSocket, cmd);
	}
}


void XDeleteBuffer(char *bname)
{
	char cmd[BUFNAMMAX + 2];
	
	if(BlistSocket != -1)
	{
		*cmd = 'D';
		strcpy(cmd + 1, bname);
		WriteSocket(BlistSocket, cmd);
	}
}


void XSwitchto(char *bname)
{
	char cmd[BUFNAMMAX + 2];
	
	if(BlistSocket != -1)
	{
		*cmd = 'H';
		strcpy(cmd + 1, bname);
		WriteSocket(BlistSocket, cmd);
	}
}


static void ListAllVars(int fd)
{
	int i;
	
	for(i = 0; i < VARNUM; ++i)
		SendVariable(fd, i);
	WriteSocket(fd, "VEdone:1");	/* done */
}

static void SendVariable(fd, i)
int fd, i;
{
	char varline[256];

	switch(Vars[i].vtype)
	{
		case STRING:
			if(Vars[i].val)
				sprintf(varline, "VS%s \"%s\"", Vars[i].vname,
					(char *)Vars[i].val);
			else
				sprintf(varline, "VS%s \"\"", Vars[i].vname);
			break;
		case DECIMAL:
			sprintf(varline, "VD%s %d", Vars[i].vname, Vars[i].val);
			break;
		case FLAG:
			sprintf(varline, "VF%s %d", Vars[i].vname, Vars[i].val);
			break;
	}
	WriteSocket(fd, varline);
}

void RunMakeCmd()
{
	int i;
	
	for(i = 0; i < MAXACCEPT; ++i)
		if(Name[i] && strcmp(Name[i], "Zmake") == 0)
		{
			extern char mkcmd[];
			char cmd[STRMAX + 4];

			sprintf(cmd, "R%s", mkcmd);
			WriteSocket(AcceptSock[i], cmd);
			return;
		}

	StartProg("Zmake");
}

int ZmakeNextErr()
{
	int i;
	
	for(i = 0; i < MAXACCEPT; ++i)
		if(Name[i] && strcmp(Name[i], "Zmake") == 0)
		{
			WriteSocket(AcceptSock[i], "N");
			return 1;
		}

	return 0;
}

static void WriteSocket(int socket, char *str)
{	/* write the string + the null character */
	register int len = strlen(str) + 1;
	
	write(socket, str, len);
}

#else
void XAddBuffer() {}
void XDeleteBuffer() {}
void CleanupSocket() {}
#endif
