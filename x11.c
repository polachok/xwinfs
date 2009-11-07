enum { ClientList, ActiveWindow, WindowDesk,
      NumberOfDesk, DeskNames, CurDesk, ELayout,
      ClientListStacking, WindowOpacity, WindowType,
      WindowTypeDesk, WindowTypeDock, WindowTypeDialog, StrutPartial, ESelTags,
      WindowName, WmName, WindowState, WindowStateFs, WindowStateModal, WindowStateHidden,
      Utf8String, TypeString, TypeWindow, TypeCardinal, TypeInteger, TypeAtom, TypeWMState, TypeWMHints, TypeWMSizeHints, Supported, NATOMS };

Atom atoms[NATOMS];

#define LASTAtom ClientListStacking

char* atomnames[NATOMS][1] = {
    { "_NET_CLIENT_LIST" },
    { "_NET_ACTIVE_WINDOW" },
    { "_NET_WM_DESKTOP" },
    { "_NET_NUMBER_OF_DESKTOPS" },
    { "_NET_DESKTOP_NAMES" },
    { "_NET_CURRENT_DESKTOP" },
    { "_ECHINUS_LAYOUT" },
    { "_NET_CLIENT_LIST_STACKING" },
    { "_NET_WM_WINDOW_OPACITY" },
    { "_NET_WM_WINDOW_TYPE" },
    { "_NET_WM_WINDOW_TYPE_DESKTOP" },
    { "_NET_WM_WINDOW_TYPE_DOCK" },
    { "_NET_WM_WINDOW_TYPE_DIALOG" },
    { "_NET_WM_STRUT_PARTIAL" },
    { "_ECHINUS_SELTAGS" },
    { "_NET_WM_NAME" },
    { "_WM_NAME" },
    { "_NET_WM_STATE" },
    { "_NET_WM_STATE_FULLSCREEN" },
    { "_NET_WM_STATE_MODAL" },
    { "_NET_WM_STATE_HIDDEN" },
    { "UTF8_STRING" },
    { "STRING" },
    { "WINDOW" },
    { "CARDINAL" },
    { "INTEGER" },
    { "ATOM" },
    { "WM_STATE" },
    { "WM_HINTS" },
    { "WM_SIZE_HINTS" },
    { "_NET_SUPPORTED" },
};

void
initatoms(void) {
    int i;
    for(i = 0; i < NATOMS; i++){
        atoms[i] = XInternAtom(dpy, atomnames[i][0], False);
    }   
}


Bool
gettextprop(Window w, Atom atom, char *text, unsigned int size);

void*
getatom(Window w, Atom atom, unsigned long *n) {
        int format, status;
        unsigned char *p = NULL;
        unsigned long tn, extra;
        Atom real;

        status = XGetWindowProperty(dpy, w, atom, 0L, 64L, False, AnyPropertyType,
                        &real, &format, &tn, &extra, (unsigned char **)&p);
        if(status == BadWindow)
                return NULL;
        if(n!=NULL)
            *n = tn;
        return p;
}

char*
atom2string(Window w, Atom a, int *n) {
        int format, status, i;
        unsigned char *p = NULL;
        unsigned long tn, extra;
        Atom type;
	Window *l;
	Atom *al;
	XWMHints *wmh;
	XSizeHints *size;
	unsigned int *c;
        char *ret = malloc(256);
        bzero(ret, 256);

        status = XGetWindowProperty(dpy, w, a, 0L, 64L, False, AnyPropertyType,
                        &type, &format, &tn, &extra, (unsigned char **)&p);
        if(status == BadWindow)
                return NULL;
        if (tn == 0)
            return NULL;
        if(type == atoms[TypeString] || type == atoms[Utf8String]) {
	    gettextprop(w, a, ret, 256);
	}
        else if(type == atoms[TypeWindow]) {
	    l = (Window*)p;
            for(i = 0; i < tn; i++)
                sprintf(ret+strlen(ret), "0x%x ", (unsigned int)l[i]);
	}
        else if(type == atoms[TypeCardinal]) {
	    c = (unsigned int*)p;
            for(i = 0; i < tn; i++)
                sprintf(ret+strlen(ret), "%d ", c[i]);
	}
	else if(type == atoms[TypeAtom]) {
	    al = (Atom*)p;
            for(i = 0; i < tn && strlen(ret) < 256; i++)
                sprintf(ret+strlen(ret), "%s ",  XGetAtomName(dpy, al[i]));
	}
	else if(type == atoms[TypeWMState]) {
	    if((long)*p == NormalState)
		sprintf(ret, "Normal");
	    else if((long)*p == IconicState)
		sprintf(ret, "Iconic");
	    else if((long)*p == WithdrawnState)
		sprintf(ret, "Withdrawn");
	}
	else if(type == atoms[TypeWMHints]) {
	    wmh = (XWMHints*)p;
	    sprintf(ret, "InputHint: %ld\nStateHint: %ld\nIconPixmapHint: %ld\n"
		    "IconWindowHint: %ld\nIconMaskHint: %ld\nWindowGroupHint: %ld\n"
		    "XUrgencyHint: %ld\nInput: %d\n", wmh->flags & InputHint,
		    wmh->flags & StateHint, wmh->flags & IconPixmapHint, wmh->flags & IconWindowHint,
		    wmh->flags & IconMaskHint, wmh->flags & WindowGroupHint, wmh->flags & XUrgencyHint, wmh->input);
	}
	else if(type == atoms[TypeWMSizeHints]) {
	    size = (XSizeHints*)p;
	    if(size->flags & PBaseSize) {
		    sprintf(ret+strlen(ret), "Base size: %dx%d\n", size->base_width, size->base_height);
	    }
	    if(size->flags & PMinSize) {
		    sprintf(ret+strlen(ret), "Minimal size: %dx%d\n", size->min_width, size->min_height);
	    }
	    if(size->flags & PResizeInc) {
		    sprintf(ret+strlen(ret), "Resize increment: %dx%d\n", size->width_inc, size->height_inc);
	    }
	    if(size->flags & PMaxSize) {
		    sprintf(ret+strlen(ret), "Maximum size: %dx%d\n", size->max_width, size->max_height);
	    }
	}
	*n = strlen(ret);
	return ret;
}

Bool
gettextprop(Window w, Atom atom, char *text, unsigned int size) {
        char **list = NULL;
        int n;
        XTextProperty name;

        if(!text || size == 0)
                return False;
        text[0] = '\0';
        if (BadWindow == XGetTextProperty(dpy, w, &name, atom))
            return False;
        if(!name.nitems)
                return False;
        if(name.encoding == XA_STRING)
                strncpy(text, (char *)name.value, size - 1);
        else { 
                if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success
                && n > 0 && *list) {
                        strncpy(text, *list, size - 1);
                        XFreeStringList(list);
                }
        }
        text[size - 1] = '\0';
        XFree(name.value);
        return True;
}
