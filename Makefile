VERSION = 1.1.2
CC = cc
CFLAGS = -Wall --std=gnu99
LDFLAGS = -shared
DBUGFLAGS = -ggdb3 -o0
SRCDIR = lib
TARGET = libstatfsext.so.$(VERSION)
SRCFILE = libstatfsext.c
SRCHEAD = libstatfsext.h
SRCOBJT = libstatfsext.o
# Edit PREFIX to install elsewhere
PREFIX = /usr
INSTDIR = lib
INCLDIR = include
LDOCDIR = doc
DOCDIR = share/man/man3

all: $(TARGET)

$(TARGET):
	$(CC) -c $(CFLAGS) -o $(SRCDIR)/$(SRCOBJT) -fpic $(SRCDIR)/$(SRCFILE)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(SRCDIR)/$(TARGET) $(SRCDIR)/$(SRCOBJT) 

test: $(TARGET)
	$(CC) -L$(SRCDIR) $(CFLAGS) -o $(SRCDIR)/test $(SRCDIR)/test.c -lstatfsext
	$(CC) -L$(SRCDIR) $(CFLAGS) -o $(SRCDIR)/test2 $(SRCDIR)/test2.c -lstatfsext
	$(CC) -L$(SRCDIR) $(CFLAGS) -o $(SRCDIR)/test3 $(SRCDIR)/test3.c -lstatfsext

strip:
	strip $(SRCDIR)/$(TARGET)

debug:
	$(CC) -c $(CFLAGS) $(DBUGFLAGS) -o $(SRCDIR)/$(SRCOBJT) -fpic $(SRCDIR)/$(SRCFILE)
	$(CC) $(LDFLAGS) $(DBUGFLAGS) -o $(SRCDIR)/$(TARGET) $(SRCDIR)/$(SRCOBJT)
	$(CC) -L$(SRCDIR) $(CFLAGS) $(DBUGFLAGS) -o $(SRCDIR)/test $(SRCDIR)/test.c -lstatfsext
	$(CC) -L$(SRCDIR) $(CFLAGS) $(DBUGFLAGS) -o $(SRCDIR)/test2 $(SRCDIR)/test2.c -lstatfsext
	$(CC) -L$(SRCDIR) $(CFLAGS) $(DBUGFLAGS) -o $(SRCDIR)/test3 $(SRCDIR)/test3.c -lstatfsext

install:
	install -o root -g root -m 0755 $(SRCDIR)/$(TARGET) $(PREFIX)/$(INSTDIR)/$(TARGET)
	install -o root -g root -m 0644 $(SRCDIR)/$(SRCHEAD) $(PREFIX)/$(INCLDIR)/$(SRCHEAD)
	install -o root -g root -m 0644 $(LDOCDIR)/getfsstat_ext.3 $(PREFIX)/$(DOCDIR)/getfsstat_ext.3
	install -o root -g root -m 0644 $(LDOCDIR)/statfs_ext.3 $(PREFIX)/$(DOCDIR)/statfs_ext.3

uninstall:
	-rm -f $(PREFIX)/$(INSTDIR)/$(TARGET)
	-rm -f $(PREFIX)/$(INCLDIR)/$(SRCHEAD)
	-rm -f $(PREFIX)/$(DOCDIR)/getfsstat_ext.3
	-rm -f $(PREFIX)/$(DOCDIR)/statfs_ext.3

clean:
	-rm -f $(SRCDIR)/$(SRCOBJT)
	-rm -f $(SRCDIR)/$(TARGET)
	-rm -f $(SRCDIR)/test
	-rm -f $(SRCDIR)/test2
	-rm -f $(SRCDIR)/test3

.PHONY: all clean install strip uninstall

