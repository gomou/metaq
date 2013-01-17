#
# $Id: libdiamond.a.mak 439 2011-09-19 01:38:28Z shijia.wxr $
#
TOPDIR  = ..

APPNAME = diamond

APPOBJS = \
		DiamondClient.o \
		UtilModule.o \

APP_CMPFLGS =

APP_LDDFLGS =

# app build type: bldexe blddll bldslb bldexe_c blddll_c bldslb_c
APP_BUILDTYPE = bldslb
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
