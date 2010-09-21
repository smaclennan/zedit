/* xdbg.c - X debug routines
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
#include <X11/X.h>

char *XEventName(int type)
{
	static char errstr[40];

	switch (type) {
	case KeyPress:		return "KeyPress";
	case KeyRelease:	return "KeyRelease";
	case ButtonPress:	return "ButtonPress";
	case ButtonRelease:	return "ButtonRelease";
	case MotionNotify:	return "MotionNotify";
	case EnterNotify:	return "EnterNotify";
	case LeaveNotify:	return "LeaveNotify";
	case FocusIn:		return "FocusIn";
	case FocusOut:		return "FocusOut";
	case KeymapNotify:	return "KeymapNotify";
	case Expose:		return "Expose";
	case GraphicsExpose:	return "GraphicsExpose";
	case NoExpose:		return "NoExpose";
	case VisibilityNotify:	return "VisibilityNotify";
	case CreateNotify:	return "CreateNotify";
	case DestroyNotify:	return "DestroyNotify";
	case UnmapNotify:	return "UnmapNotify";
	case MapNotify:		return "MapNotify";
	case MapRequest:	return "MapRequest";
	case ReparentNotify:	return "ReparentNotify";
	case ConfigureNotify:	return "ConfigureNotify";
	case ConfigureRequest:	return "ConfigureRequest";
	case GravityNotify:	return "GravityNotify";
	case ResizeRequest:	return "ResizeRequest";
	case CirculateNotify:	return "CirculateNotify";
	case CirculateRequest:	return "CirculateRequest";
	case PropertyNotify:	return "PropertyNotify";
	case SelectionClear:	return "SelectionClear";
	case SelectionRequest:	return "SelectionRequest";
	case SelectionNotify:	return "SelectionNotify";
	case ColormapNotify:	return "ColormapNotify";
	case ClientMessage:	return "ClientMessage";
	case MappingNotify:	return "MappingNotify";
	}
	sprintf(errstr, "UNKNOWN TYPE %d", type);
	return errstr;
}

/* This is tied to Zedit */
Window Zroot, zwindow;

char *XWindowName(Window window)
{
	static char wstr[80];

	if (window == zwindow)
		return "zwindow";
	else if (window == Zroot)
		return "Zroot";

	sprintf(wstr, "%lx", (unsigned long)window);
	return wstr;
}
