###############################################################
#							      #
#	Lady Heather MAKEFILE for Linux or		      #
#	OS/X (with XQuartz X11 library) 		      #
#							      #
#							      #
###############################################################

OS := $(shell uname -s)
CC = g++

#
# Linux build
#

ifeq ($(USE_SDL),1)
SDL_OBJS=heather_sdl.o SDL_gfxPrimitives.o
SDL_INC=-I$(shell sdl2-config --prefix)/include -DUSE_SDL=1
GUI_LIBS=$(shell sdl2-config --libs)
else
SDL_OBJS=
SDL_INC=
GUI_LIBS=-lX11
endif

OBJS=heather.o heathui.o heathgps.o heathmsc.o $(SDL_OBJS)

ifeq ($(OS),Linux)
INCPATH=$(SDL_INC)
WARNS = -Wno-write-strings -Wall -Wno-unused-but-set-variable -Wno-unused-variable
LDPATH=-L/usr/lib/X11
else ifeq ($(OS),Darwin)
INCPATH=$(SDL_INC) -I/usr/X11/include
WARNS = -Wno-write-strings -Wall -Wno-unused-variable
LDPATH=-L/usr/X11/lib
endif

CPPFLAGS=$(INCPATH) $(WARNS)
LDFLAGS=$(LDPATH) -lm $(GUI_LIBS)

all: heather

heather_sdl.o: heather_sdl.cpp heather.ch makefile
		  $(CC) -c heather_sdl.cpp $(CPPFLAGS)

SDL_gfxPrimitives.o: SDL_gfxPrimitives.cpp SDL_gfxPrimitives.h  makefile
		  $(CC) -c SDL_gfxPrimitives.cpp $(CPPFLAGS)

heather.o: heather.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heather.cpp $(CPPFLAGS)

heathmsc.o: heathmsc.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathmsc.cpp $(CPPFLAGS)

heathui.o: heathui.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathui.cpp $(CPPFLAGS)

heathgps.o: heathgps.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathgps.cpp $(CPPFLAGS)

heather: $(OBJS)
		  $(CC) $(OBJS) -o heather $(LDFLAGS)
clean:
		  rm -f heather $(OBJS)
