#
# Makefile.am for barrel/testsuite
#

AM_CPPFLAGS = -I$(top_srcdir)

LDADD =								\
	../barrel/libbarrel.la					\
	../barrel/Utils/libutils.la				\
	-lboost_unit_test_framework

check_PROGRAMS =						\
	getopts.test table.test raid1.test xfs1.test 		\
	complex1.test parse-line.test lvm1.test load1.test

AM_DEFAULT_SOURCE_EXT = .cc

TESTS = $(check_PROGRAMS)

EXTRA_DIST = empty1.xml empty2.xml mapping1.json load1.xml

