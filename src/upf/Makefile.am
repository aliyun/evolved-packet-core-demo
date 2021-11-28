## Process this file with automake to produce Makefile.in.

noinst_LTLIBRARIES = libupf.la

libupf_la_SOURCES = \
    upf_context.h \
	upf_gtp_msg_handler.h upf_libgtpnl.h \
    upf_init.c upf_context.c \
	upf_gtp_msg_handler.c upf_libgtpnl.c \
	upf_nl.h upf_nl.c \
	$(NULL)

libupf_la_DEPENDENCIES = \
    $(top_srcdir)/lib/fd/libfd.la \
    $(top_srcdir)/lib/gtp/libgtp.la \
    $(top_srcdir)/lib/ipfw/libipfw.la \
    $(top_srcdir)/src/common/libcommon.la \
	$(NULL)

libupf_la_LIBADD = \
    $(top_srcdir)/lib/fd/libfd.la \
    $(top_srcdir)/lib/gtp/libgtp.la \
    $(top_srcdir)/lib/ipfw/libipfw.la \
    $(top_srcdir)/src/common/libcommon.la \
	$(NULL)

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib/core/include \
    -I$(top_srcdir)/lib \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/lib/@FREEDIAMETER_DIR@/include \
	@MONGOC_CFLAGS@ \
	$(NULL)

AM_CFLAGS = \
	-Wall -Werror \
	$(NULL)

MAINTAINERCLEANFILES = Makefile.in
MOSTLYCLEANFILES = core *.stackdump
