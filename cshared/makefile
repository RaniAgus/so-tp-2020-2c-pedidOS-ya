# Project
NAME=cshared
SRC=cshared
SUBDIRS=cshared/utils
OBJ=obj

# Basic setup
CC=gcc
C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=$(OBJ)/%.o)

# Clean and compile .so
all: lib$(NAME).so

create-dirs:
	mkdir -p $(patsubst %,$(OBJ)/%,$(SUBDIRS))

lib$(NAME).so: create-dirs $(OBJS)
	$(CC) -shared -o "lib$(NAME).so" $(OBJS)

$(OBJ)/%.o: %.c
	$(CC) -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

# Add debug parameters and compile
debug: CC += -DDEBUG -g
debug: all

# Clean build files
clean:
	rm -f lib$(NAME).so
	rm -rf $(OBJ)

install: all
	sudo cp -u build/lib$(NAME).so /usr/lib
	sudo cp --parents -u $(H_SRCS) /usr/include

uninstall:
	sudo rm -f /usr/lib/lib$(NAME).so
	sudo rm -rf /usr/include/$(NAME)

.PHONY: all create-dirs clean install uninstall
