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

void
eprint(const char *errstr, ...) {
    va_list ap;

    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

Window valid_client(char *name);
static int hello_getattr(const char *path, struct stat *stbuf)
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
	if(valid_client(basename(path))) {
	    stbuf->st_mode = S_IFDIR | 0755;
	    stbuf->st_nlink = 2;
	} else if(valid_client(basename(dirname(path)))) {
	    stbuf->st_mode = S_IFREG | 0444;
	    stbuf->st_nlink = 1;
	    stbuf->st_size = 256;
	}
    }
    else
        res = -ENOENT;

    return res;
}

Window valid_client(char *name) {
    Window *list;
    Window w;
    int i;
    long unsigned int nclients;
    sscanf(name, "0x%x", (unsigned int*)&w);

    list = getatom(root, atom[ClientList], &nclients);
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
    list = getatom(root, atom[ClientList], &nclients);
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

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
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
	if(!valid_client(basename(path)))
	    return -ENOENT;
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	list_props(buf, filler, valid_client(basename(path)));
    }
    else
	return -ENOENT;

    return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
    Window w;
    char title[256];
    bzero(title, 256);
    if(strcmp(basename(path), "name") == 0)
	 if(valid_client(basename(dirname(path))))
	     return 0;

    if((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    char title[256];
    bzero(title, 256);
    Window w;
    if(strcmp(basename(path), "name") == 0) {
	 w = valid_client(basename(dirname(path)));
         if(!gettextprop(w, atom[WindowName], title, sizeof title))
		     gettextprop(w, atom[WmName], title, sizeof title);
	 size = 256;
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

static struct fuse_operations hello_oper = {
    .getattr   = hello_getattr,
    .readdir = hello_readdir,
    .open   = hello_open,
    .read   = hello_read,
};

int main(int argc, char *argv[])
{
            setlocale(LC_CTYPE, "");
	    dpy = XOpenDisplay(0);
	    if(!dpy)
		eprint("ourico: cannot open display\n");
	    screen = DefaultScreen(dpy);
	    root = RootWindow(dpy, screen);
	    initatoms();

	    return fuse_main(argc, argv, &hello_oper, NULL);
}
