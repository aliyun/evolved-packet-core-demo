## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib src support test

#bin_PROGRAMS = vepc-mmed vepc-hssd vepc-sgwd vepc-pgwd vepc-pcrfd vepc-epcd vepc-upfd vepc-spgwd
bin_PROGRAMS = vepc-mmed vepc-hssd vepc-pcrfd vepc-epcd vepc-spgwd vepc-extd

dist_vepc_mmed_SOURCES = main.c
vepc_mmed_LDADD = $(top_srcdir)/src/libmme.la

dist_vepc_hssd_SOURCES = main.c
vepc_hssd_LDADD = $(top_srcdir)/src/libhss.la

#dist_vepc_sgwd_SOURCES = main.c
#vepc_sgwd_LDADD = $(top_srcdir)/src/libsgw.la

#dist_vepc_pgwd_SOURCES = main.c
#vepc_pgwd_LDADD = $(top_srcdir)/src/libpgw.la

#dist_vepc_upfd_SOURCES = main.c
#vepc_upfd_LDADD = $(top_srcdir)/src/libupf.la

dist_vepc_extd_SOURCES = main.c
vepc_extd_LDADD = $(top_srcdir)/src/libext.la

dist_vepc_spgwd_SOURCES = main.c
vepc_spgwd_LDADD = $(top_srcdir)/src/libspgw.la

dist_vepc_pcrfd_SOURCES = main.c
vepc_pcrfd_LDADD = $(top_srcdir)/src/libpcrf.la

dist_vepc_epcd_SOURCES = main.c
vepc_epcd_LDADD = $(top_srcdir)/src/libepc.la

AM_CPPFLAGS = \
    -I$(top_srcdir)/lib/core/include \
    -I$(top_srcdir)/src \
	$(NULL)

install-data-hook:
	$(MKDIR_P) $(DESTDIR)/$(localstatedir)/log/vepc
	$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-mmed
	$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-spgwd
	$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-extd
	$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-pcrfd
	$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-hssd
	$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-epcd
#$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-upfd
#$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-sgwd
#$(MKDIR_P) $(DESTDIR)/$(localstatedir)/run/vepc-pgwd

CLEANFILES = -R data debian/test
DISTCLEANFILES = $(DIST_ARCHIVES)
MAINTAINERCLEANFILES = \
    configure config.in \
    aclocal.m4 m4/ltsugar.m4 m4/libtool.m4 m4/ltversion.m4 \
    m4/lt~obsolete.m4 m4/ltoptions.m4 \
    build-aux/ar-lib build-aux/config.guess build-aux/depcomp \
	build-aux/ltmain.sh build-aux/test-driver build-aux/compile \
	build-aux/config.sub build-aux/missing build-aux/install-sh \
    Makefile.in
MOSTLYCLEANFILES = core *.stackdump
