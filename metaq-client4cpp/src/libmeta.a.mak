#
# $Id: libncli.mak 15 2011-08-26 06:40:13Z  $
#
TOPDIR  = ..

APPNAME = meta

APPOBJS = \
		FetchConfigThread.o \
		UtilModule.o \
		Message.o \
		MessageProducer.o \
		MessageSessionFactory.o \
		Shutdownable.o \
		MetaClientConfig.o \
		RemotingClientWrapper.o \
		ZKClient.o \

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
