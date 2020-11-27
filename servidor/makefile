# Project
NAME:=servidor
SRCDIR:=src
ODIR:=obj
SRC:= $(wildcard $(SRCDIR)/*.c)
OBJ:= $(patsubst $(SRCDIR)/%.c,$(ODIR)/%.o,$(SRC))
DEPS:= $(wildcard $(SRCDIR)/*.h)

# Shared library
LIB_PATH:="../cshared/"
LIB_NAME:=cshared

# Basic setup
CC:=gcc
CFLAGS:=-g -Wall -I$(LIB_PATH)
LIBS:=-L $(LIB_PATH) -l$(LIB_NAME) -lcommons -lpthread -lreadline -lm

# Tool invocations
$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# All target
all: $(NAME)

# Clean targets
clean:
	rm -f $(NAME)
	rm -rf $(ODIR)