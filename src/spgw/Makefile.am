## Process this file with automake to produce Makefile.in.

noinst_LTLIBRARIES = libspgw.la

libspgw_la_SOURCES = \
	spgw_init.c \
    spgw_event.h spgw_event.c \
	spgw_context.h spgw_context.c \
	spgw_sm.h spgw_sm.c \
	spgw_ipfw.h spgw_ipfw.c \
	spgw_gtp_path.h spgw_gtp_path.c \
	pgw_s5c_build.h pgw_s5c_build.c \
	pgw_s5c_handler.h pgw_s5c_handler.c \
	pgw_fd_path.h pgw_fd_path.c \
	pgw_gx_handler.h pgw_gx_handler.c \
	sgw_s11_handler.h sgw_s11_handler.c \
	$(NULL)

libspgw_la_DEPENDENCIES = \
    $(top_srcdir)/lib/fd/libfd.la \
    $(top_srcdir)/lib/gtp/libgtp.la \
    $(top_srcdir)/lib/ipfw/libipfw.la \
    $(top_srcdir)/src/common/libcommon.la \
	$(NULL)

libspgw_la_LIBADD = \
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
