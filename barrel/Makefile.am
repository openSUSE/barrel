#
# Makefile.am for barrel/barrel
#

SUBDIRS = Utils

AM_CPPFLAGS = -I$(top_srcdir)

sbin_PROGRAMS = barrel

barrel_SOURCES =					\
	barrel.cc

barrel_LDADD =	 					\
	libbarrel.la

noinst_LTLIBRARIES = libbarrel.la

libbarrel_la_SOURCES =							\
	handle.h			handle.cc			\
	generic.h			generic.cc			\
	show.h				show.cc				\
	show-disks.h			show-disks.cc			\
	show-raids.h			show-raids.cc			\
	show-filesystems.h		show-filesystems.cc		\
	show-pools.h			show-pools.cc			\
	commit.h			commit.cc			\
	create-raid.h			create-raid.cc			\
	create-encryption.h		create-encryption.cc		\
	create-partition-table.h	create-partition-table.cc	\
	create-filesystem.h		create-filesystem.cc		\
	remove-device.h			remove-device.cc

libbarrel_la_LIBADD =					\
	Utils/libutils.la				\
	-lstorage-ng -lreadline

