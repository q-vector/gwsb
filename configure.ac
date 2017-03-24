# Process this file with autoconf to produce a configure script.
AC_INIT([gwsb], [0.1.0])
AM_INIT_AUTOMAKE([gwsb], [0.1.0])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_C_BIGENDIAN
AC_LANG(C++)

# Checks for libraries.
PKG_CHECK_MODULES([DENISE], [denise])
PKG_CHECK_MODULES([GTKMM_2_4], [gtkmm-3.0])
INK_REQUIRES="$INK_REQUIRES denise gtkmm-3.0"
INK_CFLAGS="$INK_CFLAGS $DENISE_CFLAGS $GTKMM_3_0_CFLAGS"
INK_LIBS="$INK_LIBS $DENISE_LIBS $GTKMM_3_0_LIBS"

# Checks for header files.
AC_CHECK_HEADERS(getopt.h)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

CPPFLAGS="$INK_CFLAGS $CPPFLAGS"
LIBS="$INK_LIBS $LIBS"

AC_SUBST(INK_REQUIRES)
AC_SUBST(INK_CFLAGS)
AC_SUBST(INK_LIBS)

AC_CONFIG_FILES([gwsb.pc Makefile src/Makefile])
AC_OUTPUT

