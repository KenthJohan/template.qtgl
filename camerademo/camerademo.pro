TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#DEFINES += GLEW_STATIC
DEFINES += CSC_SRCDIR=\\\"../camerademo/\\\"

SOURCES += main.c

LIBS += -lmingw32 -lSDL2main -lSDL2 -mwindows -lglew32 -lopengl32

