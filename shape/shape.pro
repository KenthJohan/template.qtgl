TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt



QMAKE_CFLAGS += -Wno-unused-function

SOURCES += main.c

HEADERS += csc/csc_sdlcam.h
HEADERS += csc/csc_math.h
HEADERS += csc/csc_crossos.h
HEADERS += mesh.h
HEADERS += shaper.h
INCLUDEPATH += C:/msys64/mingw64/include
INCLUDEPATH += $$PWD/nng/include

win64: LIBS += -LC:\msys64\mingw64\lib

contains(QT_ARCH, i386) {
	message("32-bit")
}else {
	message("64-bit")
	LIBS += -LC:\msys64\mingw64\lib
}

LIBS += -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32


LIBS += -L$$PWD/nng/build
LIBS += -lnng
LIBS += -lws2_32 -lmswsock -ladvapi32 -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32

