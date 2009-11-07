#define FUSE_USE_VERSION  26

#include <fuse.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

static const char *clients_path = "/clients";

Window root;
Display *dpy;
int screen;

#include "x11.c"

const char *winfs_basename(const char *path)
{
    const char *p;

    if (path == NULL)
        return NULL;

    p = strrchr(path, '/');
    if (p != NULL)
        return p + 1;
    else
        return path;
}

const char *winfs_dirname(const char *path)
{
    const char *pend, *pstart;
    char *p;

    if (path == NULL)
        return NULL;

    pend = strrchr(path, '/');
    pstart = strchr(path, '/');
    p = malloc(pend-pstart);
    bzero(p, pend-pstart);
    strncpy(p, pstart, pend-pstart);
    fprintf(stderr, ">>%s<<<\n", p);
    if (p != NULL)
        return p + 1;
    else
        return path;
}

void
eprint(const char *errstr, ...) {
    va_list ap;

    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

Window valid_client(const char *name);
static int xwinfs_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if(strcmp(path, clients_path) == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if(strncmp(path, clients_path, 7) == 0) {
	if(valid_client(winfs_basename(path))) {
	    stbuf->st_mode = S_IFDIR | 0755;
	    stbuf->st_nlink = 2;
	} else if(valid_client(winfs_basename(winfs_dirname(path)))) {
	    stbuf->st_mode = S_IFREG | 0444;
	    stbuf->st_nlink = 1;
	    stbuf->st_size = 256;
	}
    }
    else
        res = -ENOENT;

    return res;
}

Window valid_client(const char *name) {
    Window *list;
    Window w;
    int i;
    long unsigned int nclients;
    sscanf(name, "0x%x", (unsigned int*)&w);

    list = getatom(root, atoms[ClientList], &nclients);
    for(i = 0; i <= nclients; i++) {
	if(w == list[i])
	    return w;
    }
    return 0;
}

void list_clients(void *buf, fuse_fill_dir_t filler) 
{
    Window *list;
    int i;
    long unsigned int nclients;
    char name[10];
    list = getatom(root, atoms[ClientList], &nclients);
    for(i = 0; i < nclients; i++) {
	sprintf(name, "0x%x", (unsigned int)list[i]);
	filler(buf, name, NULL, 0);
    }
}

void list_props(void *buf, fuse_fill_dir_t filler, Window w) 
{
    Atom *atoms;
    char *name;
    int count, i;

    filler(buf, "name", NULL, 0);
    filler(buf, "geometry", NULL, 0);
    atoms = XListProperties(dpy, w, &count);
    for (i = 0; i < count; i++) {
       name = XGetAtomName(dpy, atoms[i]);
       filler(buf, name, NULL, 0);
    }
}

static int xwinfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;

    if(strcmp(path, "/") == 0) {
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, clients_path + 1, NULL, 0);
    }
    else if(strcmp(path, clients_path) == 0) {
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	list_clients(buf, filler);
    }
    else if(strncmp(path, clients_path, 7) == 0) {
	if(!valid_client(winfs_basename(path)))
	    return -ENOENT;
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	list_props(buf, filler, valid_client(winfs_basename(path)));
    }
    else
	return -ENOENT;

    return 0;
}

static int xwinfs_open(const char *path, struct fuse_file_info *fi)
{
    Window w;
    char title[256];
    bzero(title, 256);
    if(strcmp(winfs_basename(path), "name") == 0)
	 if(valid_client(winfs_basename(winfs_dirname(path))))
	     return 0;

    if((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int xwinfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    char title[256];
    bzero(title, 256);
    Window w;
    Atom a;
    if(strcmp(winfs_basename(path), "name") == 0) {
	 w = valid_client(winfs_basename(winfs_dirname(path)));
         if(!gettextprop(w, atoms[WindowName], title, sizeof title))
		     gettextprop(w, atoms[WmName], title, sizeof title);
	 size = 256;
    } else
    if (w = valid_client(winfs_basename(winfs_dirname(path)))) {
	a = XInternAtom(dpy, winfs_basename(path), False);
	sprintf(title, "%s", atom2string(w, a, &size));
    }

    len = strlen(title);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, title + offset, size);
    } else
        size = 0;

    return size;
}

static struct fuse_operations xwinfs_oper = {
    .getattr   = xwinfs_getattr,
    .readdir = xwinfs_readdir,
    .open   = xwinfs_open,
    .read   = xwinfs_read,
};

int main(int argc, char *argv[])
{
            setlocale(LC_CTYPE, "");
	    dpy = XOpenDisplay(0);
	    if(!dpy)
		eprint("xwinfs: cannot open display\n");
	    screen = DefaultScreen(dpy);
	    root = RootWindow(dpy, screen);
	    initatoms();

	    return fuse_main(argc, argv, &xwinfs_oper, NULL);
}

