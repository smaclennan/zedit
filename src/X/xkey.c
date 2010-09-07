#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

void main(argc, argv)
int  argc;
char *argv[];
{
	Display *display;
	int screen;
	Window root;
	XSetWindowAttributes attr;
	XWMHints wm_hints;
	XEvent event;
	KeySym key;
	char c;

	if((display = XOpenDisplay(NULL)) == NULL)
	{
		fprintf(stderr, "Cannot connect to X server %s\n", XDisplayName(NULL));
		exit(1);
	}
	screen = DefaultScreen(display);

	attr.event_mask = KeyPressMask;
	attr.background_pixel = BlackPixel(display, screen);

	root = XCreateWindow(display,  RootWindow(display, screen),
		0, 0, 100, 30, 1, CopyFromParent, InputOutput, CopyFromParent,
		CWBackPixel | CWEventMask, &attr);

	wm_hints.flags = InputHint;
	wm_hints.input = True;		/* Allow keyboard input. */
	XSetWMProperties(display, root, 0, 0, argv, argc, 0, &wm_hints, 0);

	XMapWindow(display, root);

	while(1)
	{
		XNextEvent(display, &event);

		if(event.type != KeyPress) { printf("Huh?\n"); continue; }

		XLookupString((XKeyEvent *)&event, &c, 1, &key, 0);

		if((key >= XK_space && key <= XK_asciitilde))
		{
			printf("ASCII %c\n", c);
			continue;
		}
		switch(key)
		{	/* modifiers */
			case XK_Shift_L:	printf("Left Shift\n");	break;
			case XK_Shift_R:	printf("Right Shift\n");break;
			case XK_Control_L:	printf("Left Ctrl\n");	break;
			case XK_Control_R:	printf("Right Ctrl\n"); break;
			case XK_Caps_Lock:	printf("Caps Lock\n");	break;
			case XK_Meta_L:		printf("Left Meta\n");	break;
			case XK_Meta_R:		printf("Right Meta\n");	break;
			case XK_Alt_L:		printf("Left Alt\n");	break;
			case XK_Alt_R:		printf("Right Alt\n");	break;
			case XK_Num_Lock:	printf("NumLock\n");	break;

			/* As far as I can tell these are not available on the sun */
			case XK_Shift_Lock:	printf("Shift Lock\n");	break;
			case XK_Super_L:	printf("Left Super\n");	break;
			case XK_Super_R:	printf("Right Super\n");break;
			case XK_Hyper_L:	printf("Left Hyper\n");	break;
			case XK_Hyper_R:	printf("Right Hyper\n");break;

			default:			printf(">%x\n", key);	break;
		}
	}
}
