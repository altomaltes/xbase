#
# José Angel Caso Sánchez  ( altomaltes@yahoo.es )
#
#  Jul 2004
#

AC_INIT([xbase],[6.4.1])
AC_CONFIG_SRCDIR([source/d4all.h])
AM_INIT_AUTOMAKE( [subdir-objects] )
AC_CONFIG_HEADERS( config.h )

AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE([disable])

DATE="`date '+%d %B %Y'`"

SVN_BUILD=m4_esyscmd_s([sh -c "svnversion | cut -f1 -d: "])
SVN_BUILD=m4_esyscmd_s([sh -c "./bld"])
AC_REVISION($Revision: $SVN_BUILD $)

dnl# Set some basic variables
BUILD_DATE="`date`"
AC_DEFINE_UNQUOTED(BUILD_DATE, "$BUILD_DATE", [ build date ])
AUTHORS="Jose Angel Caso Sanchez (JACS) altomaltes@yahoo.es"
AC_DEFINE_UNQUOTED(AUTHORS, "$AUTHORS", [ library authors ] )


dnl# Supply default CFLAGS, if not specified by "CFLAGS=flags ./configure"
if test ! -z "${CFLAGS}" -o ! -z "${CCOPTS}"; then
  CFLAGS_GIVEN=1
else
  CFLAGS_GIVEN=0
fi

if test -z "${CFLAGS}"; then
    if test -z "${CCOPTS}"; then
        CCOPTS="-O"
    fi
    CFLAGS="$CCOPTS"
fi

AC_ARG_WITH(cc, [  --with-cc=compiler  force libxbase to build with a particular compiler (like pgcc, egcs, etc.)],
            CC=$withval,
  if test -z "$CC"; then
    CC=check
  fi
)

if test "$CC" = "check"; then
  dnl# Check for Pentium compiler
  AC_CHECK_PROG(PGCC, pgcc, pgcc)

  if test ! -z "$PGCC"; then
    CC=$PGCC
  else
    unset CC
  fi
fi

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX

dnl# These must be run after AC_PROG_CC but before any other macros that use
dnl# the C compiler
AC_USE_SYSTEM_EXTENSIONS
AC_SEARCH_LIBS([strerror],[cposix])
LT_INIT

dnl# Where are we?
AC_MSG_CHECKING(for distribution root)
DIST_ROOT=`pwd`
AC_MSG_RESULT($DIST_ROOT)

AC_PROG_GCC_TRADITIONAL

dnl # If using gcc, use -O2.  If -g works with it, use that too
if test "${CFLAGS_GIVEN}" -ne 1; then
    if test "$GCC" = "yes"; then
        if test "$ac_cv_prog_cc_g" = "yes"; then
            CCOPTS='-g -O2'
        else
            CCOPTS='-O2'
        fi
        CFLAGS="$CCOPTS"
    fi
fi

CFLAGS+=" -fPIC -fsigned-char"
CPPFLAGS+="-I../../include"

AC_PROG_INSTALL

dnl# Check the sanity of what we've done so far
AM_SANITY_CHECK

#dnl bz2lib is required
#AC_CHECK_LIB( bz2, deflate, [LIBS="-lbz2 $LIBS" AC_DEFINE(HAVE_LIBBZ2, 1, [ Define if you have bz2lib. ])]
#            , [AC_MSG_WARN([zlib is required ])]
#            , [-lm])



dnl# If it's there, what the hell?
AM_WITH_DMALLOC

dnl# Look for needed programs
AC_CHECK_PROG( RM, rm, rm, true  )
AC_CHECK_PROG( CP, cp, cp, false )
AC_CHECK_PROG( AR, ar, ar, false )
AC_CHECK_PROG( MV, mv, mv, true  )

AC_C_BIGENDIAN

AC_C_CONST
AC_C_INLINE
#AC_PATH_XTRA

dnl# Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS( include/procesos.h signal.h errno.h fcntl.h sys/times.h sys/ioctl.h sys/select.h sys/time.h malloc.h unistd.h stdarg.h sys/filio.h sys/wait.h sys/sendfile.h sys/socket.h sys/un.h netinet/in.h netinet/ip6.h netdb.h)

dnl# Structure members
AC_CHECK_MEMBER( siginfo_t.si_utime, AC_DEFINE_UNQUOTED( SIGINFO_SI_UTIME, 1, [ Defined if siginfo has si_utime field ] ) ,,[#include<signal.h>])



dnl# Missing typedefs and replacements
AC_TYPE_MODE_T
AC_CHECK_TYPE(   off_t, long )
AC_CHECK_TYPE(  temp_t, unsigned long long )
AC_CHECK_TYPE( ticks_t, unsigned long      )
AC_CHECK_TYPE(    byte, unsigned char      )
AC_CHECK_TYPE(    word, unsigned short     )
AC_CHECK_TYPE(   dword, unsigned int       )
AC_CHECK_TYPE(   qword, unsigned long long )
AC_CHECK_TYPE(  handle, int                )
AC_CHECK_TYPE(socklen_t, int )
AC_TYPE_PID_T
AC_TYPE_UID_T

dnl# Checks for library functions.

AC_CHECK_FUNCS( atexit unsetenv setutent seteuid putenv setresuid setresgid usleep fork vsnprintf )

dnl# Portability checks for various functions
#AC_SEARCH_LIBS( getpwuid, sun )
#AC_SEARCH_LIBS( connect, socket )
#AC_SEARCH_LIBS( gethostbyname, nsl )

dnl#
dnl# Utility stuff
dnl#
dnl# Did they want debugging?
AC_MSG_CHECKING(for debugging level)
AC_ARG_WITH(debugging, [  --with-debugging[=num]  enable debugging output, num is an integer 0-9],
              if test "$withval" = "yes"; then
                  withval=4
              else
                  :
              fi
              if test "$withval" != "no"; then
                  echo "$withval"
                  AC_DEFINE_UNQUOTED(DEBUG, $withval, [ Define debug eval ] )
              else
                  echo "no debugging"
                  AC_DEFINE_UNQUOTED(DEBUG, 0, [ Define debug 0 ] )
              fi, echo "4"
                  AC_DEFINE_UNQUOTED(DEBUG, 4, [ Define debug 4 ] )
)


# LANGUAJE ENABLE
# ====
#
AC_ARG_ENABLE([language],
   AS_HELP_STRING( [[[--enable-language=@<:@none/ansi/dictionary/finnish/french/german/norwegian/scandinavian/swedish@:>@]]]
                 , [Enable database language: @<:@default=none@:>@ ( none, ansi, dictionary, finnish , french , german,  norwegian, scandinavian, swedish )] )
                 , [case "$enableval" in
      ansi)
         AC_DEFINE([S4ANSI], [0],
                   [Enable ANSI language])
        ;;

      dictionary)
         AC_DEFINE([S4DICTIONARY], [0],
                   [Enable DICTIONARY language])
        ;;

      finnish)
         AC_DEFINE([S4FINNISH], [0],
                   [Enable Finnish language])
        ;;

      french)
         AC_DEFINE([S4FRENCH], [0],
                   [Enable French language])
        ;;

      german)
         AC_DEFINE([S4GERMAN], [0],
                   [Enable German language])
        ;;

      norwegian)
         AC_DEFINE([S4NORWEGIAN], [0],
                   [Enable Nowwegian language])
        ;;

      escandinavian)
         AC_DEFINE([S4SCANDINAVIAN], [0],
                   [Enable Scandinavian language])
        ;;

      swedish)
         AC_DEFINE([S4SWEDISH], [0],
                   [Enable Swedish language])
        ;;
   esac])


# INDEX FORMAT
# ====
#

AC_ARG_ENABLE([format],
   AS_HELP_STRING( [[[--enable-format=@<:@fox/clipper/mdx@:>@]]]
                 , [Enable database index format: @<:@default=fox@:>@ ( foxpro, clipper or dbase IV mdx )] )
                 , [case "$enableval" in
      fox)
         AC_DEFINE([S4FOX], [0],
                   [Enable FOXPRO index format])
        ;;
      clipper)
         AC_DEFINE([S4CLIPPER], [],
                   [Enable CLIPPER index format])
         ;;
      mdx)
         AC_DEFINE([S4MDX], [], [Enable DBASE IV mdx format])
         ;;
      *)
         AC_MSG_ERROR([--enable-index-format=${enable_index_format}: invalid value])
   esac] )


dnl Detect windows

AM_CONDITIONAL( WINDOWS,   test "${version_type}" = "windows" )

AC_PATH_PROG([SLACKBUILD], [makepkg], [notfound])

AC_ARG_ENABLE([exdebug],
AS_HELP_STRING([--enable-exdebug=@<:@yes/no@:>@], [Extended error checking @<:@default=yes@:>@]))
AS_IF([test "x$enable_exdebug" != "xno"], AC_DEFINE([E4DEBUG], [], [Extended error checking]) )

AC_ARG_ENABLE([linkdbg],
AS_HELP_STRING([--enable-linkdbg=@<:@yes/no@:>@], [Link list error checking @<:@default=yes@:>@]))
AS_IF([test "x$enable_linkdbg" != "xno"], AC_DEFINE([E4LINK], [], [Link list error checking]) )

AC_ARG_ENABLE([miscdbg],
AS_HELP_STRING([--enable-miscdbg=@<:@yes/no@:>@], [Miscellaneous error checking @<:@default=yes@:>@]))
AS_IF([test "x$enable_miscdbg" != "xno"], AC_DEFINE([E4MISC], [], [Miscellaneous error checking]) )

AC_ARG_ENABLE([dbg],
AS_HELP_STRING([--disable-dbg=@<:@yes/no@:>@], [disable error display @<:@default=no@:>@]))
AS_IF([test "x$disable_nodbg" != "xno"], AC_DEFINE([E4OFF], [], [No error display]) )

AC_ARG_ENABLE([string],
AS_HELP_STRING([--disable-errstring=@<:@yes/no@:>@], [disable error string display @<:@default=no@:>@]))
AS_IF([test "x$disable_errstring" != "xno"], AC_DEFINE([E4OFF_STRING], [], [No error string display ]) )

AC_ARG_ENABLE([paramhigh],
AS_HELP_STRING([--enable-paramhigh=@<:@yes/no@:>@], [High-level Functional parameter error display @<:@default=yes@:>@]))
AS_IF([test "x$enable_paramhigh" != "xno"], AC_DEFINE([E4PARM_HIGH], [], [High-level Functional parameter error display]) )

AC_ARG_ENABLE([estop],
AS_HELP_STRING([--enable-estop=@<:@yes/no@:>@], [pause-on-error enabled @<:@default=no@:>@]))
AS_IF([test "x$enable_estop" != "xno"], AC_DEFINE([E4PAUSE], [], [pause-on-error enabled]) )

AC_ARG_ENABLE([ecritical],
AS_HELP_STRING([--enable-ecritical=@<:@yes/no@:>@], [Stop-on-critical-error enabled @<:@default=yes@:>@]))
AS_IF([test "x$enable_ecritical" != "xno"], AC_DEFINE([E4STOP_CRITICAL], [], [Stop-on-critical-error enabled]) )

AC_ARG_ENABLE([index],
AS_HELP_STRING([--disable-index=@<:@yes/no@:>@], [Index support source code removed=no@:>@]))
AS_IF([test "x$disable_index" != "xno"], AC_DEFINE([S4OFF_INDEX], [], [Index support source code removed]) )

AC_ARG_ENABLE([memo],
AS_HELP_STRING([--disable-memo=@<:@yes/no@:>@], [CodeBase memo support source code removed=no@:>@]))
AS_IF([test "x$disable_memo" != "xno"], AC_DEFINE([S4OFF_MEMO], [], [CodeBase memo support source code removed]) )

AC_ARG_ENABLE([multi],
AS_HELP_STRING([--disable-multi=@<:@yes/no@:>@], [Multi-user support source code removed=no@:>@]))
AS_IF([test "x$disable_multi" != "xno"], AC_DEFINE([S4OFF_MULTI], [], [Multi-user support source code removed]) )

AC_ARG_ENABLE([optimize],
AS_HELP_STRING([--disable-optimize=@<:@yes/no@:>@], [Optimization source code removed @<:@default=no@:>@]))
AS_IF([test "x$disable_optimize" != "xno"], AC_DEFINE([S4OFF_OPTIMIZE], [], [Optimization source code removed]) )


AC_ARG_ENABLE([slack],
  [AS_HELP_STRING([--enable-slack=@<:@yes/no@:>@],
    [Configure for building SLACKWARE package @<:@default=yes@:>@ (requires makepkg)])],
  [AS_IF([test "x$enable_slack" = xyes],
    [AS_IF([test "x$SLACKBUILD" = xnotfound],
      [AC_MSG_ERROR([makepkg is required for --enable-slack])],
        [$test "x$SLACKBUILD" = xnotfound],
          [AC_MSG_ERROR([makepkg is required for --enable-slack])])])],
  [enable_slack=yes])

AM_CONDITIONAL(BUILDING_SLACK, test "x$enable_slack" = "xyes" )

LDFLAGS+=-L/usr/local/lib



AC_SUBST(DIST_ROOT)
AC_SUBST(GDB_CMD_FILE)
AC_SUBST(CC)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LIBS)
AC_SUBST(SLACKBUILD)
AC_SUBST(AUTHORS)
AC_SUBST(DATE)
AC_SUBST(SVN_BUILD)

AC_CONFIG_FILES([Makefile source/Makefile ])
AC_OUTPUT

echo "
$PACKAGE $VERSION $AC_REVISION
Configuration:
--------------

  Source code location:       $srcdir
  Host System Type:           $host
  Preprocessor:               $CC $CPPFLAGS
  Compiler:                   $CC $CFLAGS
  Linker:                     $CC $LDFLAGS $LIBS
  Install path:               $prefix
  Language:                   $enable_language
  Slackware packager:         $SLACKBUILD $enable_slack

  Now type 'make' to build $PACKAGE $VERSION $SVN_BUILD.
"
