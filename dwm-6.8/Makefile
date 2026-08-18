# dwm - window manager (st/dmenu in-process; bar blocks beside this Makefile)
# See LICENSE file for copyright and license details.

include config.mk

THEME_FILES = $(wildcard themes/*)

SRC = desktop.c dwm.c drw.c util.c \
	st-0.9.3/st.c st-0.9.3/x.c \
	dmenu-5.4/dmenu.c
OBJ = desktop.o dwm.o drw.o util.o st.o stx.o dmenu.o

STCFLAGS = -I. -Ist-0.9.3 ${INCS} -DVERSION=\"0.9.3\" -D_XOPEN_SOURCE=600
DMENUCFLAGS = -I. -Idmenu-5.4 ${INCS} ${CPPFLAGS} -UVERSION -DVERSION=\"5.4\"

BATTERY = blocks/battery/battery
NET     = blocks/net/net
SPOTIFY = blocks/spotify-notify/spotify-now

all: theme_builtins.h dwm herbe-bin blocks

theme_builtins.h: ${THEME_FILES} Makefile
	@set -e; \
	{ \
		echo '/* generated from themes/ — do not edit */'; \
		echo '#ifndef THEME_BUILTINS_H'; \
		echo '#define THEME_BUILTINS_H'; \
		echo 'static const struct { const char *name; const char *body; } theme_builtins[] = {'; \
		for f in ${THEME_FILES}; do \
			name=$${f##*/}; \
			echo "	{ \"$$name\","; \
			sed 's/\\/\\\\/g; s/"/\\"/g; s/.*/		"&\\n"/' "$$f"; \
			echo '	},'; \
		done; \
		echo '};'; \
		echo '#endif'; \
	} > $@.tmp && mv $@.tmp $@

.c.o:
	${CC} -c ${CFLAGS} $<

desktop.o: desktop.c
	${CC} -c ${CFLAGS} -o $@ desktop.c

dwm.o: dwm.c config.h config.mk theme.h theme_builtins.h status.c
	${CC} -c ${CFLAGS} -o $@ dwm.c

drw.o: drw.c drw.h util.h
	${CC} -c ${CFLAGS} -o $@ drw.c

util.o: util.c util.h
	${CC} -c ${CFLAGS} -o $@ util.c

st.o: st-0.9.3/st.c st-0.9.3/st.h st-0.9.3/win.h util.h
	${CC} -c ${STCFLAGS} -o $@ st-0.9.3/st.c

stx.o: st-0.9.3/x.c st-0.9.3/st.h st-0.9.3/win.h st-0.9.3/config.h \
	config.h theme.h theme_builtins.h
	${CC} -c ${STCFLAGS} -o $@ st-0.9.3/x.c

dmenu.o: dmenu-5.4/dmenu.c dmenu-5.4/config.h config.h theme.h theme_builtins.h drw.h util.h
	${CC} -c ${DMENUCFLAGS} -o $@ dmenu-5.4/dmenu.c

config.h:
	cp config.def.h $@

dwm: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS} -lm -lrt -lutil

herbe-bin:
	${MAKE} -C herbe

blocks: ${BATTERY} ${NET} ${SPOTIFY}

${BATTERY}: blocks/battery/main.c config.h
	${CC} -Wall -Wextra -std=c23 -o $@ blocks/battery/main.c

${NET}: blocks/net/main.c config.h
	${CC} -Wall -Wextra -std=c23 -o $@ blocks/net/main.c

${SPOTIFY}: blocks/spotify-notify/main.c config.h
	${CC} -Wall -Wextra -std=c23 -o $@ blocks/spotify-notify/main.c \
		$$(pkg-config --cflags --libs dbus-1)

clean:
	rm -f dwm ${OBJ} dwm-${VERSION}.tar.gz theme_builtins.h theme_builtins.h.tmp
	rm -f ${BATTERY} ${NET} ${SPOTIFY}
	${MAKE} -C herbe clean

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R LICENSE Makefile README config.def.h config.mk\
		dwm.1 drw.h util.h desktop.c dwm.c drw.c util.c dwm.png transient.c dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dwm theme-apply ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwm ${DESTDIR}${PREFIX}/bin/theme-apply
	cp -f dmenu-5.4/dmenu_path dmenu-5.4/dmenu_path_desktop \
		${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dmenu_path \
		${DESTDIR}${PREFIX}/bin/dmenu_path_desktop
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < dwm.1 > ${DESTDIR}${MANPREFIX}/man1/dwm.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm.1
	tic -sx st-0.9.3/st.info
	${MAKE} -C herbe install
	cp -f ${BATTERY} ${NET} ${SPOTIFY} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/battery \
		${DESTDIR}${PREFIX}/bin/net \
		${DESTDIR}${PREFIX}/bin/spotify-now

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwm \
		${DESTDIR}${PREFIX}/bin/theme-apply \
		${DESTDIR}${PREFIX}/bin/dmenu_path \
		${DESTDIR}${PREFIX}/bin/dmenu_path_desktop \
		${DESTDIR}${PREFIX}/bin/battery \
		${DESTDIR}${PREFIX}/bin/net \
		${DESTDIR}${PREFIX}/bin/spotify-now \
		${DESTDIR}${MANPREFIX}/man1/dwm.1
	${MAKE} -C herbe uninstall

.PHONY: all clean dist install uninstall herbe-bin blocks
