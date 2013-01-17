MULU_META := META_$(shell date +%s)
MULU_DIAMOND := DIAMOND_$(shell date +%s)

all clean:
	-mkdir ../include/DIAMOND/
	-cp -f DiamondClient.h ../include/DIAMOND/

	$(MAKE) -f libdiamond.so.mak $@
	$(MAKE) -f libmeta.so.mak $@

	# for meta
	$(MAKE) -f libmeta.a.mak clean
	$(MAKE) -f libmeta.a.mak $@
	mkdir $(MULU_META); \
	cd $(MULU_META); \
	ar x ../../lib/liblwpr.a; \
	ar x ../../lib/libcurl.a; \
	ar x ../../lib/libzookeeper_mt.a; \
	ar x ../../lib/libmeta.a; \
	ar cru libabc.a *.o; \
	ranlib libabc.a; \
	cp -f libabc.a ../../lib/libmeta.a

	# for diamond
	$(MAKE) -f libdiamond.a.mak clean
	$(MAKE) -f libdiamond.a.mak $@
	mkdir $(MULU_DIAMOND); \
	cd $(MULU_DIAMOND); \
	ar x ../../lib/liblwpr.a; \
	ar x ../../lib/libcurl.a; \
	ar x ../../lib/libdiamond.a; \
	ar cru libabc.a *.o; \
	ranlib libabc.a; \
	cp -f libabc.a ../../lib/libdiamond.a
