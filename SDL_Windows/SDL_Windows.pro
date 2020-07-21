TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c

INCLUDEPATH += C:\msys64\mingw64\include

LIBS += -LC:\msys64\mingw64\lib

#https://stackoverflow.com/questions/115813/how-to-statically-compile-an-sdl-game-on-windows
#sdl2-config --static-libs
LIBS += -Wl,-Bstatic
LIBS += -lmingw32 -lSDL2main -lSDL2 -mwindows -Wl,--no-undefined -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -static-libgcc
LIBS += -Wl,-Bdynamic
