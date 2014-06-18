#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)

AC_INIT(Logstalgia, 1.0.6, [acaudwell@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/main.cpp])

AM_INIT_AUTOMAKE([dist-bzip2 foreign subdir-objects])

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_LANG(C++)

#Disable X11 on Macs unless required
AS_IF([test "$with_x" != yes], [
    case "$host_os" in
    darwin*)
        with_x="no"
    ;;
    esac
], [])

#GL
AX_CHECK_GL
AX_CHECK_GLU
CXXFLAGS="$CXXFLAGS $GL_CFLAGS $GLU_CFLAGS"
CPPFLAGS="$CPPFLAGS $GL_CFLAGS $GLU_CFLAGS"
LIBS="$LIBS $GL_LIBS $GLU_LIBS"

PKG_CHECK_MODULES([FT2], [freetype2 >= 9.0.3])
PKG_CHECK_MODULES([PCRE], [libpcre])
PKG_CHECK_MODULES([GLEW], [glew])

PKG_CHECK_MODULES([SDL2], [sdl2 SDL2_image], [have_sdl2=yes], [have_sdl2=no])

# use SDL 2 if available, otherwise use SDL 1.2
if test "x$have_sdl2" = xno; then
    AC_MSG_WARN([SDL 2.0 libraries not found. Trying SDL 1.2. Please note SDL 1.2 support is deprecated]) 
    PKG_CHECK_MODULES([SDL], [sdl >= 1.2.10 SDL_image])
fi

PKG_CHECK_MODULES([PNG], [libpng >= 1.2])

CPPFLAGS="${CPPFLAGS} ${FT2_CFLAGS} ${PCRE_CFLAGS} ${GLEW_CFLAGS} ${SDL2_CFLAGS} ${SDL_CFLAGS} ${PNG_CFLAGS}"
LIBS="${LIBS} ${FT2_LIBS} ${PCRE_LIBS} ${GLEW_LIBS} ${SDL2_LIBS} ${SDL_LIBS} ${PNG_LIBS}"

AC_CHECK_FUNCS([IMG_LoadPNG_RW], , AC_MSG_ERROR([SDL_image with PNG support required. Please see INSTALL]))
AC_CHECK_FUNCS([IMG_LoadJPG_RW], , AC_MSG_ERROR([SDL_image with JPEG support required. Please see INSTALL]))

#BOOST
AX_BOOST_BASE([1.46], , AC_MSG_ERROR(Boost Base >= 1.46 is required. Please see INSTALL))
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"

#GLM
AC_CHECK_HEADER([glm/glm.hpp],, AC_MSG_ERROR(GLM headers are required. Please see INSTALL))

#see if ttf-font-dir option is enabled
AC_ARG_ENABLE(ttf-font-dir,[AS_HELP_STRING([--enable-ttf-font-dir=DIR],[directory containing GNU FreeFont TTF fonts])],[sdlappfontdir="$enableval"],[sdlappfontdir=""])
AM_CONDITIONAL([FONTDIR], [test "x$sdlappfontdir" != "x"])
AC_SUBST(sdlappfontdir)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
