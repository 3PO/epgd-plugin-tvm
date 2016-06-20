#
# Makefile
#
# See the README file for copyright information and how to reach the author.
#
#

EPGD_SRC ?= ../..

include $(EPGD_SRC)/Make.config

PLUGIN = tvm
VERSION = $(shell grep 'define TVM_VERSION ' $(PLUGIN).h | awk '{ print $$3 }' | sed -e 's/[";]//g')
ARCHIVE = $(PLUGIN)-$(VERSION)-$(shell date +%Y%m%d-%H%M)

SOFILE = libepgd-tvm.so
OBJS = tvm.o 

CFLAGS += -I$(EPGD_SRC) -Wno-long-long

all: $(SOFILE)

$(SOFILE): $(OBJS)
	$(CC) $(CFLAGS) -shared $(OBJS) $(LIBS) -o $@

extst: xmltest.c
	g++ -ggdb -I$(EPGD_SRC) $(EPGD_SRC)/lib/common.c $(EPGD_SRC)/lib/config.c -lpthread xmltest.c -o extst

install:  $(SOFILE) install-config
	install -D $(SOFILE) $(_PLGDEST)/

clean:
	@-rm -f $(OBJS) core* *~ *.so extst *.xml
	rm -f ./configs/*~
	rm -f tvm-*.tgz

install-config:
	if ! test -d $(CONFDEST); then \
	   mkdir -p $(CONFDEST); \
	   chmod a+rx $(CONFDEST); \
	fi
	for i in `ls ./configs/tvmovie*.xsl`; do\
	   if ! test -f "$(CONFDEST)/$$i"; then\
	      install --mode=644 -D "$$i" $(CONFDEST)/; \
	   fi;\
	done;
	for i in `ls ./configs/tvmovie*.xml`; do\
	   if ! test -f "$(CONFDEST)/$$i"; then\
	      install --mode=644 -D "$$i" $(CONFDEST)/; \
	   fi;\
	done;

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(ARCHIVE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(ARCHIVE).tgz

#***************************************************************************
# dependencies
#***************************************************************************

tvm.o : tvm.c  tvm.h
