CC ?= cc
CFLAGS ?=
CFLAGS += -Wall
CFLAGS += `pkg-config --cflags --libs libsystemd`
#CFLAGS += -DUSE_SYSLOG
RM ?= rm -f

.PHONY: all install spotless

all: ergod

ergod:
	$(CC) $(CFLAGS) -o ergod ergod.c

install:
	sudo ./install.sh

spotless:
	$(RM) ergod
