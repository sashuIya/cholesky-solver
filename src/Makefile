# Root directory for project
ROOT_DIR=$(shell pwd)

# BUILD_ROOT is directory with object files
# BUILD_DIR is where to build
BUILD_ROOT=$(ROOT_DIR)
BUILD_DIR=$(ROOT_DIR)/../build

CC=gcc
CFLAGS=-c -Wall -O3
LDFLAGS=-lm
SOURCES=main.c array_op.c timer.c array_io.c
EXECUTABLE=a
OBJS_NAMES=$(SOURCES:.c=.o)
OBJS=$(patsubst %,$(BUILD_DIR)/%,$(OBJS_NAMES))

ALL: configure_dirs all

configure_dirs:
	@if [ ! -d $(BUILD_DIR) ] ; then \
	  mkdir -p $(BUILD_DIR) ; \
	fi

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJS_NAMES)
	$(CC) $(LDFLAGS) $(OBJS) -o $(BUILD_DIR)/$@

.c.o:
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@
