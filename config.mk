# xwinfs version
VERSION = 0.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC}  `pkg-config --cflags fuse`
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 `pkg-config --libs fuse`

# flags
CFLAGS = -Os ${INCS} -DVERSION=\"${VERSION}\" 
LDFLAGS = -s ${LIBS}
CFLAGS = -g -Wall -O0 ${INCS} -DVERSION=\"${VERSION}\"
LDFLAGS = -g ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}
#CFLAGS += -xtarget=ultra

# compiler and linker
CC = cc
