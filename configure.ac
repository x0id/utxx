#  -*- Autoconf -*-

AC_PREREQ([2.63])
AC_INIT([utxx], [0.1], [BUG-REPORT-ADDRESS])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Wno-portability])

AC_SUBST([utxxdir], [$libdir/utxx])

# Default prefix
AC_PREFIX_DEFAULT(`pwd`)
AC_SUBST([default_prefix], [${PWD}/install])
test "$prefix" = "NONE" && prefix="$default_prefix"
test "$sysconfdir" = "\${prefix}/etc" && sysconfdir='../etc'
test "$scriptdir" = "" && scriptdir='../lib'

# Setting Ix86 conditional for x86 platform
AC_CANONICAL_HOST
AM_CONDITIONAL([Ix86], [case x$host_cpu in xi?86) test -n a;; *) test;; esac])

# Options

AC_ARG_VAR([ERLC_FLAGS], [general flags to prepend to ERLC_FLAGS])

CXXFLAGS="${CXXFLAGS% } -MMD"

AC_ARG_ENABLE(debug,
     AC_HELP_STRING([--enable-debug],[enable debug [[default=yes]]]),
     [ if test "x$enable_debug" = "xyes" -o -z "x$enable_debug"; then
           # CXXFLAGS="${CXXFLAGS% } -ggdb -O0 -Wall -fno-default-inline -fno-inline -fno-inline-functions"
           CXXFLAGS="${CXXFLAGS% } -g -O0 -Wall"
       fi
     ],
)

AC_ARG_ENABLE(optimize,
     AC_HELP_STRING([--enable-optimize],[enable code optimization [[default=no]]]),
     [ if test "x$enable_optimize" = "xyes" ; then
           CXXFLAGS="${CXXFLAGS% } -O3 -g"
       fi
     ],
)

AC_ARG_ENABLE(warnings,
     AC_HELP_STRING([--enable-warnings],[enable Wall compiling [[default=yes]]]),
     [ if test "x$enable_warnings" = "xyes" -o -z "$enable_warnings"; then
           CXXFLAGS="${CXXFLAGS% } -Wall -Werror"
       fi
     ],
)

dnl optional pure compilation tests
AC_ARG_ENABLE(demo,
    AC_HELP_STRING([--enable-demo], [compile demo programs [[default=no]]]),
    [case ${enableval} in yes) demo=true;; *) demo=false;; esac], [demo=false]
)
AM_CONDITIONAL([DEMO], [test "x$demo" = "xtrue"])

# libtool
LT_INIT([disable-static])

# Checks for programs.
AC_PROG_CXX
AC_PROG_SED
AX_PROG_XSLTPROC

dnl AC_CONFIG_SRCDIR([src/common.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for libraries.

AC_CHECK_LIB(pthread, pthread_create)

AC_CHECK_FUNCS(clock_gettime, [], [
  AC_CHECK_LIB(rt, clock_gettime, [
    RTLIB="-lrt"
    AC_SUBST([RTLIB])
  ])
])

AX_BOOST_BASE([1.41.0], [], [AC_MSG_ERROR("BOOST library version >= 1.41.0 not found!")])
AX_BOOST_SYSTEM
AX_BOOST_ASIO
AX_BOOST_DATE_TIME

# workaround unexpected extra dependency on boost_system library (at least in 1.49)
LDFLAGS_SAVED_1=$LDFLAGS
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS $BOOST_SYSTEM_LIB"
AX_BOOST_THREAD
AX_BOOST_CHRONO
LDFLAGS=$LDFLAGS_SAVED_1

AX_BOOST_FILESYSTEM
AX_BOOST_UNIT_TEST_FRAMEWORK

if test x"$ac_boost_path" != x; then
    libsubdirs="lib"
    ax_arch=`uname -m`
    if test $ax_arch = x86_64 -o $ax_arch = ppc64 -o $ax_arch = s390x -o $ax_arch = sparc64; then
        libsubdirs="lib64 lib"
    fi
    for ac_boost_path_tmp in $libsubdirs; do
        if test -d "$ac_boost_path"/"$ac_boost_path_tmp" ; then
            BOOST_LIBDIR="$ac_boost_path/$ac_boost_path_tmp"
            AC_SUBST([BOOST_LIBDIR])
            break
        fi
    done
fi

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h])

# Checks for new (since 1.48) boost timer header and library
CPPFLAGS_SAVED_2=$CPPFLAGS
LDFLAGS_SAVED_2=$LDFLAGS
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([boost/timer/timer.hpp])
AC_CHECK_LIB([boost_timer], [main], [
    BOOST_TIMER_LIB="-lboost_timer $BOOST_CHRONO_LIB"
    AC_SUBST([BOOST_TIMER_LIB])
], [], [$BOOST_CHRONO_LIB $BOOST_SYSTEM_LIB])
AC_LANG_POP([C++])
CPPFLAGS=$CPPFLAGS_SAVED_2
LDFLAGS=$LDFLAGS_SAVED_2

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday socket])

dnl ------------------------------------------------------------------
dnl Output.
dnl ------------------------------------------------------------------

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([demo/Makefile])
AC_CONFIG_FILES([test.sh], [chmod +x test.sh])
AC_CONFIG_FILES([utxx.pc:utxx.pc.in])

AC_OUTPUT

echo
echo "========================================================================"
echo "   CXXFLAGS: ${CXXFLAGS}" $BOOST_CPPFLAGS
echo "      BOOST: ${BOOST_ROOT}"
echo "       UTIL: ${UTIL_ROOT}"
echo "------------------------------------------------------------------------"
echo "             Configuration completed successfully!                      "
echo "========================================================================"
echo
