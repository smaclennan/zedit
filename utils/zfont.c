/* zfont.c - Helper program to dump fixed length fonts
 * Copyright (C) 1988-2010 Sean MacLennan
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/* put out a list of the fixed length fonts */
int main(int  argc, char *argv[])
{
	XFontStruct *info;
	Display *display;
	FILE *pfp;
	char font[1024], *p;

	display = XOpenDisplay(NULL);
	if (!display) {
		fprintf(stderr, "Cannot connect to X server %s\n",
			XDisplayName(NULL));
		exit(2);
	}

	pfp = popen("xlsfonts", "r");
	if (!pfp) {
		perror("xlsfonts");
		exit(1);
	}

	while (fgets(font, 1024, pfp)) {
		p = strchr(font, '\n');
		if (p)
			*p = '\0';

		info = XLoadQueryFont(display, font);
		if (info) {
			if (info->min_bounds.width == info->max_bounds.width &&
			    font[0] == '-' && font[1] != '-')
				puts(font);
			XFree(info);
		}
	}

	pclose(pfp);
	return 0;
}


/*
 * Local Variables:
 * compile-command: "gcc -O3 -Wall zfont.c -o zfont -lX11"
 * End:
 */
