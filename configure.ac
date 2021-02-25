
AC_INIT(barrel/barrel.cc)

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

AC_SYS_LARGEFILE

AC_SUBST(VERSION)

AC_OUTPUT(
	Makefile
	barrel/Makefile
	barrel/Utils/Makefile
	testsuite/Makefile
)
