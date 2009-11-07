enum { ClientList, ActiveWindow, WindowDesk,
      NumberOfDesk, DeskNames, CurDesk, ELayout,
      ClientListStacking, WindowOpacity, WindowType,
      WindowTypeDesk, WindowTypeDock, WindowTypeDialog, StrutPartial, ESelTags,
      WindowName, WmName, WindowState, WindowStateFs, WindowStateModal, WindowStateHidden,
      Utf8String, TypeString, TypeWindow, TypeCardinal, TypeInteger, TypeAtom, Supported, NATOMS };

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
    { "_NET_SUPPORTED" },
};

void
initatoms(void) {
    int i;
    for(i = 0; i < NATOMS; i++){
        atoms[i] = XInternAtom(dpy, atomnames[i][0], False);
    }   
}

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
        char *ret = malloc(256);
        bzero(ret, 256);

        status = XGetWindowProperty(dpy, w, a, 0L, 64L, False, AnyPropertyType,
                        &type, &format, &tn, &extra, (unsigned char **)&p);
        if(status == BadWindow)
                return NULL;
        if (tn == 0)
            return NULL;
        if(type == atoms[TypeString]) {
	    sprintf(ret, "%s", p);
	}
        else if(type == atoms[TypeWindow]) {
	    l = (Window*)p;
            for(i = 0; i < tn; i++)
                sprintf(ret+strlen(ret), "0x%x ", l[i]);
	}
        else if(type == atoms[TypeCardinal]) {
            for(i = 0; i < tn; i++)
                sprintf(ret+strlen(ret), "%d ", p[i]);
	}
	else if(type == atoms[TypeAtom]) {
	    al = (Atom*)p;
            for(i = 0; i < tn && strlen(ret) < 256; i++)
                sprintf(ret+strlen(ret), "%s ",  XGetAtomName(dpy, al[i]));
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
