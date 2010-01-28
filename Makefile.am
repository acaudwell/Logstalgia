ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = logstalgia

logstalgia_SOURCES = \
	src/ball.cpp src/ball.h \
	src/core/bounds.h \
	src/core/camera.cpp src/core/camera.h \
	src/core/display.cpp src/core/display.h \
	src/core/extensions.cpp src/core/extensions.h \
	src/core/frustum.cpp src/core/frustum.h \
	src/core/fxfont.cpp src/core/fxfont.h \
	src/core/logger.cpp src/core/logger.h \
	src/core/pi.h \
	src/core/plane.cpp src/core/plane.h \
	src/core/quadtree.cpp src/core/quadtree.h \
	src/core/regex.cpp src/core/regex.h \
	src/core/resource.cpp src/core/resource.h \
	src/core/sdlapp.cpp src/core/sdlapp.h \
	src/core/seeklog.cpp src/core/seeklog.h \
	src/core/stringhash.cpp src/core/stringhash.h \
	src/core/texture.cpp src/core/texture.h \
	src/core/vectors.h \
	src/logentry.cpp src/logentry.h \
	src/logstalgia.cpp src/logstalgia.h \
	src/main.cpp src/main.h \
	src/paddle.cpp src/paddle.h \
	src/ppm.cpp src/ppm.h \
	src/requestball.cpp src/requestball.h \
	src/slider.cpp src/slider.h \
	src/summarizer.cpp src/summarizer.h \
	src/textarea.cpp src/textarea.h

CPPFLAGS = -DSDLAPP_RESOURCE_DIR=\"$(pkgdatadir)\"

dist_man_MANS = data/logstalgia.1

dist_pkgdata_DATA = data/ball.tga data/example.log data/glow.tga

if FONTDIR
CPPFLAGS += -DSDLAPP_FONT_DIR=\"$(sdlappfontdir)\"
else
fontsdir = $(pkgdatadir)/fonts
dist_fonts_DATA = data/fonts/README data/fonts/FreeMonoBold.ttf data/fonts/FreeSerif.ttf
endif
