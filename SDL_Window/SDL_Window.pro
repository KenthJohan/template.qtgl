TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

#DEFINES += main=SDL_main

LIBS += -lmingw32 -lSDL2main -lSDL2 -mwindows
