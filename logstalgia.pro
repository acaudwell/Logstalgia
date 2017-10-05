# Note: this project file only supports building on Windows with Mingw-w64
# See the INSTALL file for building instructions

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
DEFINES -= UNICODE

CONFIG += c++11
CONFIG += object_parallel_to_source

gcc {
    QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-variable -Wno-sign-compare -Wno-unused-parameter -Wno-reorder
    QMAKE_CXXFLAGS_DEBUG += -DASSERTS_ENABLED
}

mingw {
    QMAKE_CXXFLAGS += -Dmain=SDL_main
    QMAKE_LFLAGS   += -mconsole

    INCLUDEPATH += C:\msys64\mingw64\include\SDL2
    INCLUDEPATH += C:\msys64\mingw64\include\freetype2

    LIBS += -lmingw32 -lSDL2main -lSDL2.dll
    LIBS += -lSDL2_image.dll -lfreetype.dll -lpcre.dll -lpng.dll -lglew32.dll -lopengl32 -lglu32
    LIBS += -static-libgcc -static-libstdc++
    LIBS += -lcomdlg32
}

linux {
    INCLUDEPATH += /usr/include/GL        \
                   /usr/include/SDL2      \
                   /usr/include/libpng12  \
                   /usr/include/freetype2 \
                   /usr/include

    LIBS += -lGL -lGLU -lfreetype -lpcre -lGLEW -lGLU -lGL -lSDL2_image -lSDL2 -lpng12
}

VPATH += ./src

SOURCES += custom.cpp \
    logentry.cpp \
    logstalgia.cpp \
    main.cpp \
    ncsa.cpp \
    paddle.cpp \
    requestball.cpp \
    settings.cpp \
    slider.cpp \
    summarizer.cpp \
    textarea.cpp \
    src/tests.cpp \
    configwatcher.cpp \
    core/conffile.cpp \
    core/display.cpp \
    core/frustum.cpp \
    core/fxfont.cpp \
    core/logger.cpp \
    core/plane.cpp \
    core/png_writer.cpp \
    core/ppm.cpp \
    core/quadtree.cpp \
    core/regex.cpp \
    core/resource.cpp \
    core/sdlapp.cpp \
    core/seeklog.cpp \
    core/settings.cpp \
    core/shader.cpp \
    core/shader_common.cpp \
    core/stringhash.cpp \
    core/texture.cpp \
    core/timezone.cpp \
    core/vbo.cpp \
    core/vectors.cpp

HEADERS += custom.h \
    logentry.h \
    logstalgia.h \
    ncsa.h \
    paddle.h \
    requestball.h \
    settings.h \
    slider.h \
    summarizer.h \
    textarea.h \
    configwatcher.h \
    src/tests.h \
    core/bounds.h \
    core/conffile.h \
    core/display.h \
    core/frustum.h \
    core/fxfont.h \
    core/gl.h \
    core/logger.h \
    core/pi.h \
    core/plane.h \
    core/png_writer.h \
    core/ppm.h \
    core/quadtree.h \
    core/regex.h \
    core/resource.h \
    core/sdlapp.h \
    core/seeklog.h \
    core/settings.h \
    core/shader.h \
    core/shader_common.h \
    core/stringhash.h \
    core/texture.h \
    core/timezone.h \
    core/vbo.h \
    core/vectors.h \
    core/settings.h
