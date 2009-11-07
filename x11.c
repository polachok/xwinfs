
enum { ClientList, ActiveWindow, WindowDesk,
      NumberOfDesk, DeskNames, CurDesk, ELayout,
      ClientListStacking, WindowOpacity, WindowType,
      WindowTypeDesk, WindowTypeDock, WindowTypeDialog, StrutPartial, ESelTags,
      WindowName, WmName, WindowState, WindowStateFs, WindowStateModal, WindowStateHidden,
      Utf8String, Supported, NATOMS };

Atom atom[NATOMS];

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
    { "_NET_SUPPORTED" },
};

void
initatoms(void) {
    int i;
    for(i = 0; i < NATOMS; i++){
        atom[i] = XInternAtom(dpy, atomnames[i][0], False);
    }   
    XChangeProperty(dpy, root,                                                                                                                                       
                    atom[Supported], XA_ATOM, 32,
                    PropModeReplace, (unsigned char *) atom, NATOMS);
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
