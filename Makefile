all: xwinsize

CC = gcc
CFLAGS = -O2
LDFLAGS = -lX11
PREFIX = /usr/local

SRC = xwinsize.c
OBJ = ${SRC:.c=.o}

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

xwinsize: ${OBJ}
	$(CC) -o xwinsize ${OBJ} $(CFLAGS) $(LDFLAGS)

clean:
	@echo cleaning
	rm -f xwinsize ${OBJ}

install:
	@echo installing xwinsize to ${DESTDIR}${PREFIX}/bin...
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f xwinsize ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/xwinsize

uninstall:
	@echo removing xwinsize from ${DESTDIR}{PREFIX}/bin
	rm -f ${DESTDIR}${PREFIX}/bin/xwinsize

.PHONY: all clean install uninstall
