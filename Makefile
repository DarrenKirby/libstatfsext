CC = cc
CFLAGS ?= -Wall -Werror --std=gnu99
LDFLAGS = -shared
DBUGFLAGS = -ggdb3 -o0
SRCDIR = lib
TARGET = libstatfsext.so
SRCFILE = libstatfsext.c
SRCHEAD = libstatfsext.h
SRCOBJT = libstatfsext.o
INSTDIR = /usr/lib
INCLDIR = /usr/include

all: $(TARGET)

$(TARGET):
	$(CC) -c $(CFLAGS) -o $(SRCDIR)/$(SRCOBJT) -fpic $(SRCDIR)/$(SRCFILE)
	$(CC) $(LDFLAGS) -o $(SRCDIR)/$(TARGET) $(SRCDIR)/$(SRCOBJT) 

test: $(TARGET)
	$(CC) -L$(SRCDIR) $(CFLAGS) -o $(SRCDIR)/test $(SRCDIR)/main.c -lstatfsext

strip:
	strip $(SRCDIR)/$(TARGET)

debug:
	$(CC) -c $(CFLAGS) $(DBUGFLAGS) -o $(SRCDIR)/$(SRCOBJT) -fpic $(SRCDIR)/$(SRCFILE)
	$(CC) $(LDFLAGS) -o $(SRCDIR)/$(TARGET) $(SRCDIR)/$(SRCOBJT)

install:
	install -o root -g root -m 0755 $(SRCDIR)/$(TARGET) $(INSTDIR)/$(TARGET)
	install -o root -g root -m 0644 $(SRCDIR)/$(SRCHEAD) $(INCLDIR)/$(SRCHEAD)

uninstall:
	-rm -rf $(INSTDIR)$(TARGET)
	-rm -rf $(INCLDIR)/$(SRCHEAD)

clean:
	-rm -rf $(SRCDIR)/libstatfsext.o
	-rm -rf $(SRCDIR)/libstatfsext.so
	-rm -rf $(SRCDIR)/test

.PHONY: all clean install strip

