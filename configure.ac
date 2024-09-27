AC_INIT

AC_CONFIG_SRCDIR([barrel/barrel.cc])

VERSION=`cat ./VERSION`

CWARNS="-Wall -Wextra -Wformat -Wmissing-prototypes"
CFLAGS="${CFLAGS} ${CWARNS}"
CXXWARNS="-Wall -Wextra -Wformat -Wnon-virtual-dtor -Wno-unused-parameter -Wsuggest-override"
CXXFLAGS="${CXXFLAGS} -std=c++17 ${CXXWARNS}"

AM_INIT_AUTOMAKE(barrel, $VERSION)
AM_CONFIG_HEADER(config.h)

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CXX

LT_INIT

AC_PREFIX_DEFAULT(/usr)

AC_PATH_PROG([XSLTPROC], [xsltproc], [/usr/bin/xsltproc])

AC_SYS_LARGEFILE

PKG_CHECK_MODULES(XML2, libxml-2.0)
PKG_CHECK_MODULES(JSON_C, json-c, [], [AC_MSG_WARN([Cannot find json-c. Please install libjson-c-devel])])

AC_SUBST(VERSION)

AC_CONFIG_FILES([
	Makefile
	barrel/Makefile
	barrel/Utils/Makefile
	data/Makefile
	doc/Makefile
	doc/barrel.xml:doc/barrel.xml.in
	testsuite/Makefile
	testsuite/helpers/Makefile
	package/barrel.spec:barrel.spec.in
	po/Makefile
])

AC_OUTPUT
