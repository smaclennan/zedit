#include <tk.h>
#include <string.h>
#include <stdlib.h>

/* XErrorProc -- Toe-hold for debugging X Protocol botches. */
static int XErrorProc(ClientData data, XErrorEvent *errEventPtr)
{
	Tk_Window w = (Tk_Window)data;
	fprintf(stderr, "X protocol error: ");
	fprintf(stderr, "error=%d request=%d minor=%d\n",
		errEventPtr->error_code, errEventPtr->request_code,
		errEventPtr->minor_code);
	/* Claim to have handled the error. */
	return 0;
}

/* A table for command line arguments. */
static char *display = NULL;
static int ParentPid = 0;

Tk_ArgvInfo argTable[] = {
	{"-display",TK_ARGV_STRING, 0, (char *)&display,   "Display to use"},
	{"-pid",	TK_ARGV_INT,	0, (char *)&ParentPid, "Parent pid"},
	{"", TK_ARGV_END,}
};

extern int sock;
int SocketSend(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int ParseMakeLine(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
void ReadSocket();

main(int argc, char *argv[])
{
	Tcl_Interp *interp;
	Tk_Window mainWindow;
	char *trace, *app, *zpath;
	char tclname[128];

	/* get the application name */
	if((app = strrchr(argv[0], '/')) == 0) app = argv[0];

	interp = Tcl_CreateInterp();
	if (Tk_ParseArgv(interp, (Tk_Window) NULL, &argc, argv,
		argTable, 0) != TCL_OK) {
		fprintf(stderr, "%s\n", interp->result);
		exit(1);
	}

	if(ParentPid == 0)
	{
		printf("usage: %s -pid parent_pid [args...]\n", argv[0]);
		exit(1);
	}
	if((zpath = getenv("ZPATH")) == 0)
	{
		printf("ZPATH not set.\n");
		exit(1);
	}

	OpenSocket(ParentPid, interp, app);

	/*
	 * Create the main window. The name of the application
	 * for use with the send command is "myapp". The
	 * class of the application for X resources is "Myapp".
	 */
	if((mainWindow = Tk_CreateMainWindow(interp, display, app, app)) == NULL)
	{
		fprintf(stderr, "%s\n", interp->result);
		exit(1);
	}
	/*
	 * Register the X protocol error handler, and ask for
	 * a synchronous protocol to help debugging.
	 */
	Tk_CreateErrorHandler(Tk_Display(mainWindow), -1, -1, -1,
		XErrorProc, (ClientData)mainWindow);

	/*
	 * Grab an initial size and background.
	 */
	Tk_GeometryRequest(mainWindow, 200, 200);
	Tk_SetWindowBackground(mainWindow,
		WhitePixelOfScreen(Tk_Screen(mainWindow)));

	/*
	 * This is where Tcl_AppInit would be called.
	 * In this case, we do the work right here.
	 */

	if (Tcl_Init(interp) != TCL_OK)
	{
		fprintf(stderr, "Tcl_Init failed: %s\n", interp->result);
		exit(2);
	}
	if (Tk_Init(interp) != TCL_OK)
	{
		fprintf(stderr, "Tk_Init failed: %s\n", interp->result);
		exit(2);
	}

	/* which script to use - we don't want to modify argv[0] (app) */
	sprintf(tclname, "%s/%c%s.tcl",
		zpath,
		isupper(*app) ? tolower(*app) : *app,
		app + 1);
	
	Tcl_CreateCommand(interp, "socketsend", SocketSend,
		(ClientData)Tk_MainWindow(interp), (Tcl_CmdDeleteProc *)NULL);

	/* This is for Zmake only but what the hell... */	
	Tcl_CreateCommand(interp, "parseoutput", ParseMakeLine,
		(ClientData)Tk_MainWindow(interp), (Tcl_CmdDeleteProc *)NULL);
	
	Tk_CreateFileHandler(sock, TK_READABLE, ReadSocket, 0);

	if(Tcl_EvalFile(interp, tclname) != TCL_OK)
	{
		fprintf(stderr, "%s: %s\n", tclname, interp->result);
		if((trace = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY)) != NULL)
		{
			fprintf(stderr, "*** TCL TRACE ***\n%s\n", trace);
			exit(2);
		}
	}

	while(1) Tk_DoOneEvent(TK_ALL_EVENTS);
}
