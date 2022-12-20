
VERSION = 20180302

MAJOR = 2
MINOR = 0
TINY = 0

HOME = .
top_builddir = $(HOME)

prefix = /usr/local
exec_prefix = ${prefix}
datarootdir = ${prefix}/share
ETC	= ${DESTDIR}${prefix}/etc
BIN	= ${DESTDIR}${exec_prefix}/bin
DATAROOT = ${DESTDIR}${prefix}/share
SHARE = ${DESTDIR}${datarootdir}/lorcon/
MAN = ${DESTDIR}${datarootdir}/man
LIB = ${DESTDIR}${exec_prefix}/lib
INCLUDE = ${DESTDIR}${prefix}/include

CC = gcc
LDFLAGS =  -L$(LIB)
LIBS =  -lpcap -lm -L/usr/lib/x86_64-linux-gnu -lnl-genl-3 -lnl-3 
CFLAGS = -I./   -DHAVE_CONFIG_H -g -O2 -I/usr/include/libnl3  -DLORCON_VERSION=$(VERSION) -I$(INCLUDE)
SHELL = /bin/bash
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LTCOMPILE = $(LIBTOOL) --mode=compile $(CC) $(CFLAGS)

DEPEND = .depend

LIBOBJ = ifcontrol_linux.lo iwcontrol.lo madwifing_control.lo nl80211_control.lo \
		wifi_ht_channels.lo \
		 lorcon_packet.lo lorcon_packasm.lo lorcon_forge.lo \
		 drv_mac80211.lo drv_tuntap.lo drv_madwifing.lo drv_file.lo \
		 sha1.lo \
		 lorcon.lo lorcon_multi.lo 
LIBOUT = liborcon2.la

TXTESTOBJ = tx.o
TXTESTOUT = tx

TXTUNOBJ  = tools/tuntx.o
TXTUNOUT  = tools/tuntx

MCSSWEEP2OBJ = tools/mcs_sweep2.o
MCSSWEEP2OUT = tools/mcs_sweep2

all:	$(DEPEND) $(LIBOUT) 

$(LIBOUT):	$(LIBOBJ)
	$(LIBTOOL) --mode=link $(CC) $(LDFLAGS) $(LIBS) -o $(LIBOUT) $(LIBOBJ) \
					  -rpath $(LIB) -release $(MAJOR).$(MINOR).$(TINY)

$(TXTESTOUT):	$(TXTESTOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TXTESTOUT) $(TXTESTOBJ) $(LIBS) -lorcon2

$(TXTUNOUT):	$(TXTUNOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TXTUNOUT) $(TXTUNOBJ) $(LIBS) -lorcon2 -lpcap

$(MCSSWEEP2OUT):	$(MCSSWEEP2OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(MCSSWEEP2OUT) $(MCSSWEEP2OBJ) $(LIBS) -lorcon2 

tools:	$(LIBOUT) $(TXTUNOUT) $(L2PINGOUT) $(MCSSWEEP2OUT)

install:	$(LIBOUT)
	install -d -m 755 $(LIB)
	$(LIBTOOL) --mode=install install -c $(LIBOUT) $(LIB)/$(LIBOUT)
	install -d -m 755 $(INCLUDE)
	install -d -m 755 $(INCLUDE)/lorcon2/
	install -m 644 lorcon.h $(INCLUDE)/lorcon2/lorcon.h
	install -m 644 lorcon_packet.h $(INCLUDE)/lorcon2/lorcon_packet.h
	install -m 644 lorcon_packasm.h $(INCLUDE)/lorcon2/lorcon_packasm.h
	install -m 644 lorcon_forge.h $(INCLUDE)/lorcon2/lorcon_forge.h
	install -m 644 lorcon_multi.h $(INCLUDE)/lorcon2/lorcon_multi.h
	install -m 644 ieee80211.h $(INCLUDE)/lorcon2/lorcon_ieee80211.h
	install -d -m 755 $(MAN)/man3
	install -o root -m 644 lorcon.3 $(MAN)/man3/lorcon.3

	$(LDCONFIG)

clean:
	@-rm -f *.o
	@-rm -f *.lo
	@-rm -f *.la
	@-rm -rf .libs
	@-rm -f $(TXTESTOUT)
	@-rm -f $(MCSSWEEP2OUT)
	@-rm -f $(TXTUNOUT)

distclean:
	@-$(MAKE) clean
	@-rm -f *~
	@-rm cscope.out
	@-rm -f $(DEPEND)
	@-rm -f config.status
	@-rm -f config.h
	@-rm -f config.log
	@-rm -f Makefile

dep:
	@$(MAKE) depend

depend:
	@$(MAKE) $(DEPEND)

$(DEPEND):
	@-rm -f $(DEPEND)
	@echo "Generating dependencies... "
	@echo > $(DEPEND)
	@$(CC) $(CFLAGS) -MM \
		`echo $(LIBOBJ) | sed -e "s/\.lo/\.c/g"` >> $(DEPEND)

include $(DEPEND)

.c.o:	$(DEPEND)
	$(CC) $(CFLAGS) -c $*.c -o $@ 

.c.lo:	$(DEPEND)
	$(LTCOMPILE) -c $*.c -o $@

.SUFFIXES: .c .o .lo


