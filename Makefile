# xwinfs
# © 2009 Alexander Polakov

include config.mk

SRC = xwinfs.c
OBJ = ${SRC:.c=.o}

all: options xwinfs

options:
	@echo xwinfs build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

xwinfs: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f xwinfs ${OBJ} xwinfs-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p xwinfs-${VERSION}
	@cp -R xwinfsrc LICENSE Makefile xwinfs.1 README config.mk config.h ${SRC} xwinfs-${VERSION}
	@tar -cf xwinfs-${VERSION}.tar xwinfs-${VERSION}
	@gzip xwinfs-${VERSION}.tar
	@rm -rf xwinfs-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp xwinfs ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/xwinfs
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < xwinfs.1 > ${DESTDIR}${MANPREFIX}/man1/xwinfs.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/xwinfs.1
	@echo install xwinfsrc to ${DESTDIR}${PREFIX}/share/examples/xwinfs
	@mkdir -p ${DESTDIR}${PREFIX}/share/examples/xwinfs
	@cp xwinfsrc ${DESTDIR}${PREFIX}/share/examples/xwinfs

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/xwinfs
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/xwinfs.1
	@echo removing xwinfsrc from  ${DESTDIR}${PREFIX}/share/examples/xwinfs
	@rm -rf ${DESTDIR}${PREFIX}/share/examples/xwinfs

.PHONY: all options clean dist install uninstall
