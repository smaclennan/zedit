/* SAM This is old obsolete code that shows how to use X client messages. */

#include <X11/Xatom.h>

Atom z_pipe;

	/* create the  atom */
	z_pipe = XInternAtom(display, "Z_PIPE", False);


	/* process the client event */
			case ClientMessage:
				if(event.xclient.message_type == z_pipe)
				{
					processpipe(&event.xclient);
					Refresh();
				}
#if DBG
				else
				{
					int i;
	
					Dbg("GOT UNKNOWN CLIENT MESSAGE: %x\nFORMAT: %d\n\t",
						event.xclient.message_type, event.xclient.format);
					for(i = 0; i < 5; ++i)
						Dbg("%x  ", event.xclient.data.l[i]);
					Dbg('\n');
				}
#endif


/* This was xpipe.c */

#include <X11/Xlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "z.h"

extern Display *display;
extern Window Zroot;
extern Atom z_pipe;

/* This routine is called when the forked process gets a SIGHUP */
static FILE *pfp;
static Boolean ClosePipe;

/* SAM Only one "shell" command at a time can run! */
static Buffer *pbuff = NULL;

void Die()
{
	ClosePipe = TRUE;
}


int Dopipe(buff, cmd)
Buffer *buff;
char *cmd;
{
    XClientMessageEvent ev;
	char str[1024], *p;
	int i, n;

	if(buff->child != EOF || pbuff)
	{	/* should never happen */
		Tbell();
		return 0;
	}

	pbuff = buff;
	if((buff->child = fork()) == 0)
	{
		ClosePipe = FALSE;
		signal(SIGHUP, Die);

		sprintf(str, "%s 2>&1", cmd);
		if((pfp = popen(str, "r")) == NULL) exit(666);

	    ev.type			= ClientMessage;
		ev.window		= Zroot;
		ev.message_type	= z_pipe;
		ev.format		= 8;
		while(fgets(p = str, 1024, pfp))
		{
			n = strlen(str);
			while(n > 0)
			{
				i = n > 19 ? 19 : n;
				ev.data.b[19] = i;
				memcpy(ev.data.b, p, i);
				XSendEvent(display, Zroot, False, 0L, (XEvent *)&ev);
				n -= i;
				p += i;
			}
			XFlush(display);	/* flush once per line */
			if(ClosePipe) break;
		}

		ev.format		= 32;
		ev.data.l[0]	= pclose(pfp);
		XSendEvent(display, Zroot, False, 0L, (XEvent *)&ev);
		XFlush(display);

		exit(0);	/* done */
	}
	
	return buff->child != EOF;
}


void processpipe(event)
XClientMessageEvent *event;
{
	extern int NextErrorCalled;
	Buffer *save;
	Mark mark;
	int i;

	if(pbuff)
		if(event->format == 32)
		{	/* done */
			if(event->data.l[0] == 0)
				Message(pbuff, "Done.");
			else
			{
				sprintf(PawStr, "Exit %d.", event->data.l[0] >> 8);
				Message(pbuff, PawStr);
				CallZmake();
			}
			/* SAM We should check for other children */
			while((i = wait(NULL)) != -1 && i != pbuff->child) ;
			pbuff->child = EOF;
			pbuff->bmodf = FALSE;
			if(strcmp(pbuff->bname, MAKEBUFF) == 0) Tbell();
			pbuff = NULL;
		}
		else
		{	/* send to make buffer */
			save = Curbuff;
			Bswitchto(pbuff);
			Bmrktopnt(&mark);
			Btoend();
			for(i = 0; i < event->data.b[19]; ++i)
				Binsert(event->data.b[i]);
			if(NextErrorCalled) Bpnttomrk(&mark);
			Bswitchto(save);
			Refresh();
		}
#if DBG
	else Dbg("Spurious Z_INVOKER ClientMessage.");
#endif
}


void Unvoke(child, check)
Buffer *child;
Boolean check;
{
	if(child && child->child != EOF)
	{
		kill(child->child, SIGHUP);
		if(check) wait(NULL);
	}
}
#include "z.h"

#ifndef PIPESH
#error file
#include <X11/Xlib.h>
#include <signal.h>
#include <sys/wait.h>

extern Display *display;
extern Window Zroot;
extern Atom z_pipe;

/* This routine is called when the forked process gets a SIGHUP */
static FILE *pfp;
static Boolean ClosePipe;

/* SAM Only one "shell" command at a time can run! */
static Buffer *pbuff = NULL;

void Die()
{
	ClosePipe = TRUE;
}


int Dopipe(buff, cmd)
Buffer *buff;
char *cmd;
{
    XClientMessageEvent ev;
	char str[1024], *p;
	int i, n;

	if(buff->child != EOF || pbuff)
	{	/* should never happen */
		Tbell();
		return 0;
	}

	pbuff = buff;
	if((buff->child = fork()) == 0)
	{
		ClosePipe = FALSE;
		signal(SIGHUP, Die);

		sprintf(str, "%s 2>&1", cmd);
		if((pfp = popen(str, "r")) == NULL) exit(666);

	    ev.type			= ClientMessage;
		ev.window		= Zroot;
		ev.message_type	= z_pipe;
		ev.format		= 8;
		while(fgets(p = str, 1024, pfp))
		{
			n = strlen(str);
			while(n > 0)
			{
				i = n > 19 ? 19 : n;
				ev.data.b[19] = i;
				memcpy(ev.data.b, p, i);
				XSendEvent(display, Zroot, False, 0L, (XEvent *)&ev);
				n -= i;
				p += i;
			}
			XFlush(display);	/* flush once per line */
			if(ClosePipe) break;
		}

		ev.format		= 32;
		ev.data.l[0]	= pclose(pfp);
		XSendEvent(display, Zroot, False, 0L, (XEvent *)&ev);
		XFlush(display);

		exit(0);	/* done */
	}
	
	return buff->child != EOF;
}


void processpipe(event)
XClientMessageEvent *event;
{
	extern int NextErrorCalled;
	Buffer *save;
	Mark mark;
	int i;

	if(pbuff)
		if(event->format == 32)
		{	/* done */
			if(event->data.l[0] == 0)
				Message(pbuff, "Done.");
			else
			{
				sprintf(PawStr, "Exit %d.", event->data.l[0] >> 8);
				Message(pbuff, PawStr);
				CallZmake();
			}
			/* SAM We should check for other children */
			while((i = wait(NULL)) != -1 && i != pbuff->child) ;
			pbuff->child = EOF;
			pbuff->bmodf = FALSE;
			if(strcmp(pbuff->bname, MAKEBUFF) == 0) Tbell();
			pbuff = NULL;
		}
		else
		{	/* send to make buffer */
			save = Curbuff;
			Bswitchto(pbuff);
			Bmrktopnt(&mark);
			Btoend();
			for(i = 0; i < event->data.b[19]; ++i)
				Binsert(event->data.b[i]);
			if(NextErrorCalled) Bpnttomrk(&mark);
			Bswitchto(save);
			Refresh();
		}
#if DBG
	else Dbg("Spurious Z_INVOKER ClientMessage.");
#endif
}


void Unvoke(child, check)
Buffer *child;
Boolean check;
{
	if(child && child->child != EOF)
	{
		kill(child->child, SIGHUP);
		if(check) wait(NULL);
	}
}


/* Invoke a shell on the other end of a two way pipe. */
int Pinvoke(argv, in, out)
char *argv[];
FILE **in, **out;
{
	int from[2], to[2];
	int child;

	if(pipe(from) == 0)
		if(pipe(to) == 0)
		{
			if((child = fork()) == 0 )
			{	/* child */
				(void)close(from[0]);
				(void)close(to[1]);
				dup2(to[0],   0);
				dup2(from[1], 1);
				dup2(from[1], 2);
				execvp(argv[0], argv);
				printf("Unable to exec %s\n", argv[0]);
				exit(666);
			}
		
			(void)close(from[1]);		/* we close these fail or not */
			(void)close(to[0]);
			if(child != EOF)
			{
				*in  = fdopen(from[0], "r");
				*out = fdopen(to[1], "w");
				if(*in && *out) return child;
			}
			else
				Echo( "Unable to fork shell" );

			(void)close(from[0]);
			(void)close(to[1]);
		}
		else
		{
			(void)close(from[0]);
			(void)close(from[1]);
		}
	Echo("Unable to open pipes");
	return EOF;
}

int Checkpipes(type)
int type;
{
	return 0;
}
#endif
