CC = cc
CFLAGS = -Wall -ggdb3 -o0 --std=gnu99
LDFLAGS = -shared
DBUGFLAGS = -ggdb3 -o0
SRCDIR = lib
TARGET = libstatfsext.so
SRCFILE = libstatfsext.c
SRCHEAD = libstatfsext.h
SRCOBJT = libstatfsext.o
INSTDIR = /usr/lib
INCLDIR = /usr/include
LDOCDIR = doc
DOCDIR = /usr/share/man/man3

all: $(TARGET)

$(TARGET):
	$(CC) -c $(CFLAGS) -o $(SRCDIR)/$(SRCOBJT) -fpic $(SRCDIR)/$(SRCFILE)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(SRCDIR)/$(TARGET) $(SRCDIR)/$(SRCOBJT) 

test: $(TARGET)
	$(CC) -L$(SRCDIR) $(CFLAGS) -o $(SRCDIR)/test $(SRCDIR)/test.c -lstatfsext
	$(CC) -L$(SRCDIR) $(CFLAGS) -o $(SRCDIR)/test2 $(SRCDIR)/test2.c -lstatfsext

strip:
	strip $(SRCDIR)/$(TARGET)

debug:
	$(CC) -c $(CFLAGS) $(DBUGFLAGS) -o $(SRCDIR)/$(SRCOBJT) -fpic $(SRCDIR)/$(SRCFILE)
	$(CC) $(LDFLAGS) -o $(SRCDIR)/$(TARGET) $(SRCDIR)/$(SRCOBJT)

install:
	install -o root -g root -m 0755 $(SRCDIR)/$(TARGET) $(INSTDIR)/$(TARGET)
	install -o root -g root -m 0644 $(SRCDIR)/$(SRCHEAD) $(INCLDIR)/$(SRCHEAD)
	install -o root -g root -m 0644 $(LDOCDIR)/getfsstat_ext.3 $(DOCDIR)/getfsstat_ext.3
	install -o root -g root -m 0644 $(LDOCDIR)/statfs_ext.3 $(DOCDIR)/statfs_ext.3

uninstall:
	-rm -rf $(INSTDIR)/$(TARGET)
	-rm -rf $(INCLDIR)/$(SRCHEAD)
	-rm -rf $(DOCDIR)/getfsstat_ext.3
	-rm -rf $(DOCDIR)/statfs_ext.3

clean:
	-rm -rf $(SRCDIR)/libstatfsext.o
	-rm -rf $(SRCDIR)/libstatfsext.so
	-rm -rf $(SRCDIR)/test
	-rm -rf $(SRCDIR)/test2

.PHONY: all clean install strip

