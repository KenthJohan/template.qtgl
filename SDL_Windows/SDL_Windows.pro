TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c

INCLUDEPATH += C:\msys64\mingw64\include

win32: LIBS += -LC:\msys64\mingw64\lib -lmingw32 -lSDL2main -lSDL2
