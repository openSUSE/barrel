#
# Makefile.am for barrel/barrel
#

SUBDIRS = Utils

AM_CPPFLAGS = -I$(top_srcdir) $(XML2_CFLAGS)

sbin_PROGRAMS = barrel

barrel_SOURCES =					\
	barrel.cc

barrel_LDADD =	 					\
	libbarrel.la

noinst_LTLIBRARIES = libbarrel.la

libbarrel_la_SOURCES =							\
	handle.h			handle.cc			\
	cmds.h				cmds.cc				\
	help.h				help.cc				\
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
	remove-pool.h			remove-pool.cc			\
	rename-pool.h			rename-pool.cc			\
	extend-pool.h			extend-pool.cc			\
	reduce-pool.h			reduce-pool.cc			\
	remove-device.h			remove-device.cc		\
	load-devicegraph.h		load-devicegraph.cc		\
	save-devicegraph.h		save-devicegraph.cc

libbarrel_la_LIBADD =					\
	Utils/libutils.la				\
	-lstorage-ng -lreadline

