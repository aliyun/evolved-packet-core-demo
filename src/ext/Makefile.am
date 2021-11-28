## Process this file with automake to produce Makefile.in.

noinst_LTLIBRARIES = libext.la

libext_la_SOURCES = \
    ext_init.c \
    ext_context.c ext_context.h \
    ext_msg_handler.c ext_msg_handler.h \
    ext_data_syn.c ext_data_syn.h \
	$(NULL)

libext_la_DEPENDENCIES = \
    $(top_srcdir)/lib/fd/libfd.la \
    $(top_srcdir)/src/common/libcommon.la \
	$(NULL)

libext_la_LIBADD = \
    $(top_srcdir)/lib/fd/libfd.la \
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
