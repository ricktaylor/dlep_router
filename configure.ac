AC_PREREQ(2.61)

AC_INIT(dlep_router,0.1.1)

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([autoheader_dummy.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile])

AM_INIT_AUTOMAKE([foreign 1.10 no-define subdir-objects])

# Use the latest silent rules if possible
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE
AC_CANONICAL_HOST

AC_PROG_CC

# Turn on all warnings and errors
CFLAGS="$CFLAGS -pedantic -std=c89 -Wall"

AC_OUTPUT
