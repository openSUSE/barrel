#
# Makefile.am for barrel/barrel/Utils
#

AM_CPPFLAGS = -I$(top_srcdir) $(XML2_CFLAGS)

noinst_LTLIBRARIES = libutils.la

libutils_la_SOURCES =			\
	GetOpts.h	GetOpts.cc	\
	Args.h				\
	Table.h		Table.cc	\
	Colors.h	Colors.cc	\
	Text.h		Text.cc		\
	JsonFile.h	JsonFile.cc	\
	Readline.h	Readline.cc	\
	Prompt.h	Prompt.cc	\
	Misc.h		Misc.cc		\
	Mockup.h	Mockup.cc	\
	BarrelTmpl.h			\
	BarrelDefines.h

libutils_la_LIBADD =			\
	-lstorage-ng -ljson-c
