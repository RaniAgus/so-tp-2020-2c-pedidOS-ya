# Library name and subfolders
LIBRARY_NAME:=cshared
UTILSDIR:=./cshared/utils
SRCDIR:=./cshared

# Basic setup
CC:=gcc
CFLAGS:= -I$(UTILSDIR) -I$(SRCDIR) -g -Wall -fPIC -shared
LIBS:=-lcommons -lpthread

# Source files
CSHARED_SRC += $(wildcard $(UTILSDIR)/*.c)
CSHARED_SRC += $(wildcard $(SRCDIR)/*.c)

# Dependencies
CSHARED_DEPS += $(LIBRARY_NAME).h
CSHARED_DEPS += $(wildcard $(UTILSDIR)/*.h)
CSHARED_DEPS += $(wildcard $(SRCDIR)/*.h)

# Tool invocations
lib$(LIBRARY_NAME).so: $(CSHARED_SRC) $(CSHARED_DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# All target
all: lib$(LIBRARY_NAME).so

# Clean targets
clean:
	rm *.so