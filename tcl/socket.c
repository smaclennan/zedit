#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <tk.h>

extern int errno;
#define FATAL(s)	{ perror(s); exit(1); }

static Tcl_Interp *TclInterp;
static fd_set Socket;
int sock = -1;


void OpenSocket(int parentpid, Tcl_Interp *interp, char *app)
{
	struct sockaddr outSock;
	char init[20];

	TclInterp = interp;

	if((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) FATAL("socket");

	outSock.sa_family = AF_UNIX;
	sprintf(outSock.sa_data, "/tmp/.zbl%04x", parentpid);
	if(connect(sock, &outSock, sizeof(outSock)) == -1) FATAL("connect");

	FD_ZERO(&Socket);
	FD_SET(sock, &Socket);

	sprintf(init, "I%s", app);
	write(sock, init, strlen(init) + 1);
}


void ReadSocket()
{
	fd_set readfds = Socket;
	struct timeval poll;
	
	poll.tv_sec = poll.tv_usec = 0;
	if(select(sock + 1, &readfds, 0, 0, &poll) > 0)
	{
		char buf[1024], *p;
		char cmd[2048];
		int n;
		
		if((n = read(sock, buf, 1024)) == 0)
			/* socket died and so must we */
			exit(0);
		else if(n == -1)
			/* error - just ignore it? */
			return;
			
		p = buf;
		while(n > 0)
		{
			switch(*p)
			{
			case 'A':
				sprintf(cmd, "AddList %s", p + 1);
				Tcl_Eval(TclInterp, cmd);
				break;
			case 'D':
				sprintf(cmd, "DelList %s", p + 1);
				Tcl_Eval(TclInterp, cmd);
				break;
			case 'H':
				sprintf(cmd, "Highlight %s", p + 1);
				Tcl_Eval(TclInterp, cmd);
				break;
			case 'N':
				sprintf(cmd, "NextError", 10);
				Tcl_Eval(TclInterp, cmd);
				break;
			case 'R':
				sprintf(cmd, "Run \"%s\"", p + 1);
				Tcl_Eval(TclInterp, cmd);
				break;
			case 'V':
				sprintf(cmd, "VarAdd %c %s", *(p + 1), p + 2);
				Tcl_Eval(TclInterp, cmd);
				break;
			}

			n -= strlen(p) + 1;
			p += strlen(p) + 1;
		}
	}
}

/* Called from the tcl script */
int SocketSend(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
	int len;
	
	if(argc != 2)
	{
		interp->result = "Usage: sendsocket string";
		return TCL_ERROR;
	}

	len = strlen(argv[1]) + 1;
	if(write(sock, argv[1], len) != len)
	{
		interp->result = "socketsend failed";
		return TCL_ERROR;
	}

	return TCL_OK;
}
