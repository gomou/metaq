#
# $Id: makefile 3 2011-08-19 02:25:45Z  $
#

SHELL    = /bin/sh

TOPDIR  := $(shell pwd)/..

TARGETS_DIR  = \
		$(TOPDIR)/src \

all::
	@-mkdir $(TOPDIR)/include/lwpr
	@-cp $(TOPDIR)/src/*.h $(TOPDIR)/include/lwpr/
	@-cp $(TOPDIR)/src/lwpr.h $(TOPDIR)/include/
	@for tsubdir in $(TARGETS_DIR); \
	do \
		echo "making $${tsubdir}"; \
		( cd $${tsubdir} && $(MAKE) ) || exit 1; \
	done

clean::
	@for tsubdir in $(TARGETS_DIR); \
	do \
		echo "clean $${tsubdir}"; \
		( cd $${tsubdir} && $(MAKE) clean ) || exit 1; \
	done
