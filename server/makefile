# Project
NAME:=server
SRCDIR:=src
ODIR:=obj
IDIR:=src

SRC:= $(wildcard $(SRCDIR)/*.c)
OBJ:= $(patsubst $(SRCDIR)/%.c,$(ODIR)/%.o,$(SRC))
DEPS:= $(wildcard $(IDIR)/*.h)

# Shared library
LIB_PATH:="../shared/src"
LIB_NAME:=cshared

# Basic setup
CC:=gcc
CFLAGS=-g -Wall -I$(LIB_PATH)
LIBS:=-L $(LIB_PATH) -l$(LIB_NAME) -lcommons -lpthread -lreadline

BINS:=$(NAME)

# Tool invocations
$(LIB_PATH)/lib$(LIB_NAME).so:
	-cd $(LIB_PATH) && $(MAKE)

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJ) $(LIB_PATH)/lib$(LIB_NAME).so
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)


# All target
all: $(BINS)

# Release target
release: CFLAGS += -DRELEASE
release: clean $(BINS)

# Clean targets
clean:
	rm -f $(ODIR)/*.o
	rm -f $(NAME)
