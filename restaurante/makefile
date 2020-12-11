# Project
NAME:=restaurante
SRC=src
SUBDIRS=src
OBJ=obj

# Shared library
LIB_NAME:=cshared
LIB_PATH:="../$(LIB_NAME)/"
LIBS:=-L $(LIB_PATH) -l$(LIB_NAME) -lcommons -lpthread -lreadline -lm

# Basic setup
CC:=gcc
C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=$(OBJ)/%.o)

# Clean and compile project
all: $(NAME)

create-dirs:
	mkdir -p $(patsubst %,$(OBJ)/%,$(SUBDIRS))

shared-library:
	-cd $(LIB_PATH) && make

$(NAME): create-dirs shared-library $(OBJS)
	$(CC) -I$(LIB_PATH) -g -Wall -o "$(NAME)" $(OBJS) $(LIBS)

$(OBJ)/$(SRC)/%.o: $(SRC)/%.c
	$(CC) -I$(LIB_PATH) -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

# Clean build files
clean:
	rm -f $(NAME)
	rm -rf $(OBJ)