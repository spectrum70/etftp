# PC (x86_64) makefile, used for host-side dev. test

CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++

BIN=etftp
LIBS=

# relative paths
SRCDIR=src
INCDIR=include
OBJDIR=obj
BINDIR=.

SRCS:=$(wildcard $(SRCDIR)/*.cc)
OBJS:=$(patsubst %.cc,%.o,$(SRCS))
OBJS:=$(patsubst $(SRCDIR)%,$(OBJDIR)%,$(OBJS))

CXXFLAGS=-c -pipe -fomit-frame-pointer -Wall -std=c++17 -DGIT_VERSION="\"$(shell git log -1 --pretty='%h')\""
#CXXFLAGS+=-DHOSTPC_BUILD

DEBUG ?= 0
ifeq ($(DEBUG), 1)
CXXFLAGS+=-O0 -ggdb -DDEBUG
else
CXXFLAGS+=-O3
endif

.PHONY: all $(library)

all: $(BINDIR)/$(BIN)

$(BINDIR)/$(BIN): $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cc | $(OBJDIR)
	$(CXX) -I$(INCDIR) $(CXXFLAGS) $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

install: $(BINDIR)/$(BIN)
	cp $(BINDIR)/$(BIN) $(CONFIG_PREFIX)/usr/bin

clean:
	rm $(OBJS)
	rm $(BIN)
