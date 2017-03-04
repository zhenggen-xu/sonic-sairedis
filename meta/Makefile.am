AM_CPPFLAGS = -I/usr/include/sai

if DEBUG
DBGFLAGS = -ggdb -D_DEBUG_
else
DBGFLAGS = -g
endif

lib_LTLIBRARIES = libmeta.la libsaimetadata.la

# we are not adding CPPFLAGS since they are added
# automatic to each C preprocessor, and currently
# we add -std=c++11 flag to CFLAGS_COMMON which
# will cause compilaton error on C files
# and we already confirmed that SAI/meta files
# can be compiled with success
#
# NOTE: this compilation could be moved to configure.ac

libmeta_la_SOURCES =  \
					  ../SAI/meta/saimetadata.c \
					  ../SAI/meta/saimetadatautils.c

libmeta_la_CFLAGS = $(DBGFLAGS)

../SAI/meta/saimetadata.h ../SAI/meta/saimetadata.c:
	make -C ../SAI/meta

clean-local:
	make -C ../SAI/meta clean

libsaimetadata_la_SOURCES = \
							sai_extra_acl.cpp \
							sai_extra_buffer.cpp \
							sai_extra_mirror.cpp \
							sai_extra_qosmaps.cpp \
							sai_extra_queue.cpp \
							sai_extra_schedulergroup.cpp \
							sai_extra_scheduler.cpp \
							sai_extra_tunnel.cpp \
							sai_extra_udf.cpp \
							sai_extra_wred.cpp \
							sai_meta.cpp \
							sai_meta_sanity.cpp \
							saiattributelist.cpp \
							saiserialize.cpp

libsaimetadata_la_CPPFLAGS = $(DBGFLAGS) $(AM_CPPFLAGS) $(CFLAGS_COMMON)
libsaimetadata_la_LIBADD = -lhiredis -lswsscommon libmeta.la

bin_PROGRAMS = tests

tests_SOURCES = tests.cpp
tests_CPPFLAGS = $(DBGFLAGS) $(AM_CPPFLAGS) $(CFLAGS_COMMON)
tests_LDADD = -lhiredis -lswsscommon -lpthread -L$(top_dir)/meta/.libs -lsaimetadata

TESTS = tests
