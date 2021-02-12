TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += CSC_SRCDIR=\\\"../demo2/\\\"
#DEFINES += DEBUG

QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-field-initializers

SOURCES += demo3.c



LIBS += -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32 -lSDL2_net

LIBS += -lws2_32 -lwsock32 -lpthread
