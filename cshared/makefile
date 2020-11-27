# Library name and subfolders
LIBRARY_NAME:=cshared
UTILSDIR:=./cshared/utils
SRCDIR:=./cshared

# Basic setup
CC:=gcc
CFLAGS:=-I$(UTILSDIR) -I$(SRCDIR) -g -Wall -fPIC -shared
LIBS:=-lcommons -lpthread

# Source files
CSHARED_SRC:=$(wildcard $(UTILSDIR)/*.c) $(wildcard $(SRCDIR)/*.c)

# Dependencies
CSHARED_DEPS:=$(wildcard $(UTILSDIR)/*.h) $(wildcard $(SRCDIR)/*.h)

# Tool invocations
lib$(LIBRARY_NAME).so: $(CSHARED_SRC) $(CSHARED_DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# All target
all: lib$(LIBRARY_NAME).so

install: all
	cp -u lib$(LIBRARY_NAME).so /usr/lib
	cp --parents -u $(CSHARED_DEPS) /usr/include

uninstall:
	rm -f /usr/lib/$(LIBRARY_NAME).so
	rm -rf /usr/include/$(LIBRARY_NAME)

# Clean targets
clean:
	rm *.so