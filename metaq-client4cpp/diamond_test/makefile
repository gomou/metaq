#
# $Id: makefile 538 2011-09-30 07:36:30Z shijia.wxr $
#

TOPDIR = ..

all: client

clean:
	rm -f *.o
	rm -f client

client: client.cpp
	g++ -o $@ $< $(TOPDIR)/lib/libdiamond.a -I$(TOPDIR)/include -L$(TOPDIR)/lib -lz -lcrypt -lldap -lrt -lidn
