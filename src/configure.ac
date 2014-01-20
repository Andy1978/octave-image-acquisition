#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Octave-Forge image-acquisition package], [0.1.0], [andy.weber.aw@gmail.com])

# Checks for programs.
AC_PROG_CXX
AC_LANG(C++)

AC_CHECK_PROG([HAVE_MKOCTFILE], [mkoctfile], [yes], [no])
if [test $HAVE_MKOCTFILE = "no"]; then
  AC_MSG_ERROR([mkoctfile required to install $PACKAGE_NAME])
fi

AC_PATH_PROG([FLTK_CONFIG], [fltk-config], [no])

if [test $FLTK_CONFIG = "no"]; then
  AC_MSG_ERROR([fltk-config required to install $PACKAGE_NAME])
else
  LIBS="`fltk-config --ldflags` $LIBS"
fi
    
# Checks for libraries.
AC_CHECK_LIB(v4l2, v4l2_open, [], [
  AC_MSG_ERROR([unable to find the v4l2_open() function])
])

# Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/time.h])

# check for v4l2 headers
AC_CHECK_HEADERS([libv4l2.h linux/videodev2.h],[],[
  AC_MSG_ERROR([unable to find the needed v4l2 headers])
])

# check for fltk headers
AC_CHECK_HEADERS([FL/Fl.H FL/Fl_Double_Window.H FL/fl_draw.H],[],[
  AC_MSG_ERROR([unable to find the needed fltk headers])
])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([munmap select strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_NOTICE([

$PACKAGE_NAME is now configured with

   LIBS:   $LIBS
])
