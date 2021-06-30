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
	show-lvm-vgs.h			show-lvm-vgs.cc			\
	show-filesystems.h		show-filesystems.cc		\
	show-pools.h			show-pools.cc			\
	show-commit.h			show-commit.cc			\
	commit.h			commit.cc			\
	create-raid.h			create-raid.cc			\
	create-lvm-vg.h			create-lvm-vg.cc		\
	create-lvm-lv.h			create-lvm-lv.cc		\
	create-encryption.h		create-encryption.cc		\
	create-partition-table.h	create-partition-table.cc	\
	create-filesystem.h		create-filesystem.cc		\
	create-pool.h			create-pool.cc			\
	extend-pool.h			extend-pool.cc			\
	reduce-pool.h			reduce-pool.cc			\
	remove-device.h			remove-device.cc		\
	load.h				load.cc				\
	save.h				save.cc

libbarrel_la_LIBADD =					\
	Utils/libutils.la				\
	-lstorage-ng -lreadline

