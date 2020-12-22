TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += CSC_SRCDIR=\\\"../demo2/\\\"

QMAKE_CFLAGS += -Wno-unused-function

SOURCES += main.c

LIBS += -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32

