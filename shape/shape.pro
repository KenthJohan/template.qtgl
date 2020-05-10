TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -Wno-unused-function

SOURCES += main.c
SOURCES += glad.c

HEADERS += glad.h
HEADERS += khrplatform.h
HEADERS += csc/csc_sdlcam.h
HEADERS += csc/csc_math.h
HEADERS += csc/csc_crossos.h
HEADERS += mesh.h
HEADERS += shaper.h
INCLUDEPATH += C:\msys64\mingw64\include


win32: LIBS += -LC:\msys64\mingw64\lib -lmingw32 -lSDL2main -lSDL2 -lopengl32
