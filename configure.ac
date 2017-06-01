# Process this file with autoconf to produce a configure script.
#AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_INIT(MEDICI, 0.1.9, david@disintegration.org)
AC_CONFIG_SRCDIR([src/adt.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lexpat':
AC_CHECK_LIB([expat],
	     [XML_ExpatVersion],,
             AC_MSG_WARN([support for XML TSGs will be disabled]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove memset strdup])

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 examples/Makefile
                 src/Makefile])
AC_OUTPUT
