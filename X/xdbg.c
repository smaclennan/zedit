#include <X11/X.h>

char *XEventName(int type)
{
	static char errstr[40];
	
	switch(type)
	{
		case KeyPress:			return "KeyPress";
		case KeyRelease:		return "KeyRelease";
		case ButtonPress:		return "ButtonPress";
		case ButtonRelease:		return "ButtonRelease";
		case MotionNotify:		return "MotionNotify";
		case EnterNotify:		return "EnterNotify";
		case LeaveNotify:		return "LeaveNotify";
		case FocusIn:			return "FocusIn";
		case FocusOut:			return "FocusOut";
		case KeymapNotify:		return "KeymapNotify";
		case Expose:			return "Expose";
		case GraphicsExpose:	return "GraphicsExpose";
		case NoExpose:			return "NoExpose";
		case VisibilityNotify:	return "VisibilityNotify";
		case CreateNotify:		return "CreateNotify";
		case DestroyNotify:		return "DestroyNotify";
		case UnmapNotify:		return "UnmapNotify";
		case MapNotify:			return "MapNotify";
		case MapRequest:		return "MapRequest";
		case ReparentNotify:	return "ReparentNotify";
		case ConfigureNotify:	return "ConfigureNotify";
		case ConfigureRequest:	return "ConfigureRequest";
		case GravityNotify:		return "GravityNotify";
		case ResizeRequest:		return "ResizeRequest";
		case CirculateNotify:	return "CirculateNotify";
		case CirculateRequest:	return "CirculateRequest";
		case PropertyNotify:	return "PropertyNotify";
		case SelectionClear:	return "SelectionClear";
		case SelectionRequest:	return "SelectionRequest";
		case SelectionNotify:	return "SelectionNotify";
		case ColormapNotify:	return "ColormapNotify";
		case ClientMessage:		return "ClientMessage";
		case MappingNotify:		return "MappingNotify";
	}
	sprintf(errstr, "UNKNOWN TYPE %d", type);
	return errstr;
}


/* This is tied to Zedit */
Window Zroot, zwindow;
#ifdef BORDER3D
Window textwindow, scrollwindow, PAWwindow;
#endif

char *XWindowName(Window window)
{
	static char wstr[80];
	
	if(window == zwindow)
		return "zwindow";
	else if(window == Zroot)
		return "Zroot";
#ifdef BORDER3D
	else if(window == textwindow)
		return "textwindow";
	else if(window == scrollwindow)
		return "scrollwindow";
	else if(window == PAWwindow)
		return "PAWwindow";
#endif

	sprintf(wstr, "%x", window);
	return wstr;
}
		
