#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/* put out a list of the fixed length fonts */
void main(argc, argv)
int  argc;
char *argv[];
{
	XFontStruct *info;
	Display *display;
	FILE *pfp;
	char font[1024], *p;

	if((display = XOpenDisplay(NULL)) == NULL)
	{
		fprintf(stderr, "Cannot connect to X server %s\n", XDisplayName(NULL));
		exit(2);
	}

	if((pfp = popen("xlsfonts", "r")) == 0)
	{
		perror("xlsfonts");
		exit(1);
	}
	
	while(fgets(font, 1024, pfp))
	{
		if((p = strchr(font, '\n')) != 0) *p = '\0';

		if((info = XLoadQueryFont(display, font)) != NULL)
		{
			if(info->min_bounds.width == info->max_bounds.width &&
				font[0] == '-' && font[1] != '-')
					puts(font);
			XFree(info);
		}
	}
	
	pclose(pfp);
	
	exit(0);
}
