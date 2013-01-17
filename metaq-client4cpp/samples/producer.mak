#
# $Id: libncli.mak 15 2011-08-26 06:40:13Z  $
#
TOPDIR  = ..

APPNAME = producer

APPOBJS = \
		producer.o \


APP_CMPFLGS =

APP_LDDFLGS = $(TOPDIR)/lib/libmeta.a -lz -lcrypt -lldap -lrt -lidn

# app build type: bldexe blddll bldslb bldexe_c blddll_c bldslb_c
APP_BUILDTYPE = bldexe
all: $(APP_BUILDTYPE)
clean: cleanup

bldexe blddll bldslb bldexe_c blddll_c bldslb_c cleanup:
	@TOPDIR="$(TOPDIR)"; \
	APPNAME="$(APPNAME)"; \
	APPOBJS="$(APPOBJS)"; \
	APP_CMPFLGS="$(APP_CMPFLGS)"; \
	APP_LDDFLGS="$(APP_LDDFLGS)"; \
	APP_BUILDTYPE="$(APP_BUILDTYPE)"; \
	export TOPDIR APPNAME APPOBJS APP_CMPFLGS APP_LDDFLGS APP_BUILDTYPE; \
	$(MAKE) -f $(TOPDIR)/project/makstand $@
