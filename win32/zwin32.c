/* zwin32.c - portability functions for Zedit
 * Copyright (C) 2013-2017 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "z.h"

static void psepfixup(char *path)
{
	while ((path = strchr(path, '\\')) != NULL)
		*path = '/';
}

/* Fixup the pathname. 'to' and 'from' cannot overlap.
 * Currently just a trivial version.
 */

int pathfixup(char *to, char *from)
{
	/* If there is a drive letter... assume fully rooted */
	if (isalpha(*from) && *(from + 1) == ':')
		strcpy(to, from);
	/* Also assume if it starts with a slash it is rooted */
	else if (*from == '/' || *from == '\\')
		strcpy(to, from);
	else {
		getcwd(to, PATHMAX);
		strcat(to, "/");
		strcat(to, from);
	}

	psepfixup(to);
	strlwr(to);

	return 0;
}

char *zgetcwd(char *dir, int len)
{
	*dir = '\0';

	if (!getcwd(dir, len - 1))
		return NULL;

	psepfixup(dir);
	strlwr(dir);

	if (*(dir + strlen(dir) - 1) != '/')
		strcat(dir, "/");

	return dir;
}

char *gethomedir(void)
{
	static char home[PATHMAX];
	char *homedrive = getenv("HOMEDRIVE");
	char *homepath = getenv("HOMEPATH");

	if (!homedrive || !homepath)
		return NULL;

	snprintf(home, sizeof(home), "%s%s", homedrive, homepath);
	psepfixup(home);

	return home;
}

static void mouse_scroll(int row, bool down)
{
	struct wdo *wdo = wfind(row);
	if (!wdo) {
		error("Not on a window."); /* XEmacs-ish */
		return;
	}

	wswitchto(wdo);

	Arg = 3;
	down ? Znext_line() : Zprevious_line();
}

static void mouse_point(int row, int col, bool set_mark)
{
	int atcol;
	struct mark tmark;
	struct wdo *wdo = wfind(row);
	if (!wdo) {
		error("Not on a window."); /* XEmacs-ish */
		return;
	}

	if (wdo != Curwdo) {
		wswitchto(wdo);
		/* We need Prow and Pcol to be correct. */
		zrefresh();
	}

	bmrktopnt(Bbuff, &tmark);

	/* Move the point to row */
	if (row > Prow)
		while (Prow < row) {
			bcsearch(Bbuff, '\n');
			++Prow;
		}
	else if (row <= Prow) {
		while (Prow > row) {
			bcrsearch(Bbuff, '\n');
			--Prow;
		}
		tobegline(Bbuff);
	}

	/* Move the point to col */
	atcol = 0;
	while (col > 0 && !bisend(Bbuff) && Buff() != '\n') {
		int n = chwidth(Buff(), atcol, false);
		bmove1(Bbuff);
		col -= n;
		atcol += n;
	}

	if (set_mark) {
		Zset_mark(); /* mark to point */
		bpnttomrk(Bbuff, &tmark); /* reset mark */
	}
}

static _inline void do_mouse(MOUSE_EVENT_RECORD *mouse)
{
	if (mouse->dwEventFlags & MOUSE_WHEELED) {
		mouse_scroll(mouse->dwMousePosition.Y,
				 mouse->dwButtonState & 0x80000000);
		zrefresh();
	} else if (mouse->dwButtonState & 0xffff) {
		mouse_point(mouse->dwMousePosition.Y,
				mouse->dwMousePosition.X,
				mouse->dwButtonState & RIGHTMOST_BUTTON_PRESSED);
		zrefresh();
	}
}

static int z_input_cb(INPUT_RECORD *input)
{
	switch (input->EventType) {
	case KEY_EVENT: /* 1 */
		/* handled in tgetkb() */
		break;
	case WINDOW_BUFFER_SIZE_EVENT:
		Rowmax = input->Event.WindowBufferSizeEvent.dwSize.Y;
		Colmax = input->Event.WindowBufferSizeEvent.dwSize.X;
		Zredisplay();		/* update the windows */
		zrefresh();		/* force a screen update */
		break;
	case MOUSE_EVENT:
		do_mouse(&input->Event.MouseEvent);
		break;
	}

	return 0;
}

void os_init(void)
{
	winkbd_event_cb = z_input_cb;
}
