#
# Makefile.am for barrel/testsuite
#

SUBDIRS = helpers .

AM_CPPFLAGS = -I$(top_srcdir) $(XML2_CFLAGS)

LDADD =								\
	../barrel/libbarrel.la					\
	../barrel/Utils/libutils.la				\
	-lboost_unit_test_framework

check_PROGRAMS =						\
	getopts.test table.test raid1.test xfs1.test 		\
	complex1.test parse-line.test lvm1.test load1.test	\
	pools1.test pools2.test show1.test show2.test		\
	show3.test remove1.test remove2.test			\
	luks1.test gpt1.test misuse1.test misuse2.test		\
	misuse3.test ext1.test btrfs1.test stack1.test

AM_DEFAULT_SOURCE_EXT = .cc

TESTS = $(check_PROGRAMS)

EXTRA_DIST = empty1.xml empty2.xml empty3.xml 			\
	mapping1.json msdos1.xml load1.xml			\
	real1.xml real2.xml real3.xml real4.xml real5.xml 	\
	real6.xml dmraid1.xml dmraid2.xml mapping2.json

