ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = logstalgia

logstalgia_CXXFLAGS = -std=gnu++0x -Wall -Wno-sign-compare -Wno-reorder -Wno-unused-but-set-variable -Wno-unused-variable

logstalgia_SOURCES = \
	src/ncsa.cpp \
	src/ball.cpp \
	src/core/conffile.cpp \
	src/core/display.cpp \
	src/core/frustum.cpp \
	src/core/fxfont.cpp \
	src/core/logger.cpp \
	src/core/ppm.cpp \
	src/core/png_writer.cpp \
	src/core/plane.cpp \
	src/core/quadtree.cpp \
	src/core/regex.cpp \
	src/core/resource.cpp \
	src/core/sdlapp.cpp \
	src/core/seeklog.cpp \
	src/core/settings.cpp \
	src/core/shader.cpp \
	src/core/shader_common.cpp \
	src/core/stringhash.cpp \
	src/core/texture.cpp \
	src/core/timezone.cpp \
	src/core/vbo.cpp \
	src/core/vectors.cpp \
	src/custom.cpp \
	src/logentry.cpp \
	src/logstalgia.cpp \
	src/main.cpp \
	src/paddle.cpp \
	src/requestball.cpp \
	src/settings.cpp \
	src/slider.cpp \
	src/summarizer.cpp \
	src/textarea.cpp

AM_CPPFLAGS = -DSDLAPP_RESOURCE_DIR=\"$(pkgdatadir)\"

dist_pkgdata_DATA = data/ball.tga data/example.log data/glow.tga

install-data-hook:
	mkdir -p -m 755 ${DESTDIR}/$(mandir)/man1
	$(SED) 's|SDLAPP_RESOURCE_DIR|$(pkgdatadir)|g' data/logstalgia.1 | gzip -f9 > $(DESTDIR)$(mandir)/man1/logstalgia.1.gz

uninstall-hook:
	-rm -f $(DESTDIR)$(mandir)/man1/logstalgia.1.gz

if FONTDIR
AM_CPPFLAGS += -DSDLAPP_FONT_DIR=\"$(sdlappfontdir)\"
else
fontsdir = $(pkgdatadir)/fonts
dist_fonts_DATA = data/fonts/README data/fonts/FreeMonoBold.ttf data/fonts/FreeSerif.ttf
endif
