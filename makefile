###############################################################
#							      #
#	Lady Heather MAKEFILE for Linux, FreeBSD, or	      #
#	OS/X (with XQuartz X11 library) 		      #
#							      #
#							      #
###############################################################

# Run make with USE_SDL=1 set to build with SDL support
# instead of X11 for Mac and Linux:
# bash$ USE_SDL=1 make

OS := $(shell uname -s)
CC = g++

ifeq ($(USE_SDL),1)
SDL_OBJS=SDL_gfxPrimitives.o heather_sdl.o
SDL_INC=-I$(shell sdl2-config --prefix)/include -DUSE_SDL=1
SDL_LIBS=$(shell sdl2-config --static-libs)
else
SDL_OBJS=
SDL_INC=
SDL_LIBS=
endif

#
# Linux build
#

ifeq ($(OS),Linux)

# WARNS = -Wno-write-strings
WARNS = -Wno-write-strings -Wall -Wno-unused-but-set-variable -Wno-unused-variable -Wno-format-overflow
ifeq ($(USE_SDL),1)
CPPFLAGS=$(SDL_INC)
LDFLAGS=-$(SDL_LIBS)
else
CPPFLAGS=-I/opt/X11/include
LDFLAGS=-L/usr/X11/lib -lm -lX11
endif

all: heather

heather.o: heather.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heather.cpp $(WARNS) $(CPPFLAGS)

heathmsc.o: heathmsc.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathmsc.cpp $(WARNS) $(CPPFLAGS)

heathui.o: heathui.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathui.cpp $(WARNS) $(CPPFLAGS)

heathgps.o: heathgps.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathgps.cpp $(WARNS) $(CPPFLAGS)

heather: heather.o heathmsc.o heathui.o heathgps.o $(SDL_OBJS)
		  $(CC) heather.o heathui.o heathgps.o heathmsc.o $(SDL_OBJS) -o heather $(LDFLAGS)

SDL_gfxPrimitives.o: SDL_gfxPrimitives.cpp SDL_gfxPrimitives.h  makefile
		  $(CC) -c SDL_gfxPrimitives.cpp $(WARNS) $(CPPFLAGS)

heather_sdl.o: heather_sdl.cpp heather.ch makefile
		  $(CC) -c heather_sdl.cpp $(WARNS) $(CPPFLAGS)

clean:
		  rm heather.o heathui.o heathgps.o heathmsc.o $(SDL_OBJS) heather

endif


#
# OS/X build
#

ifeq ($(OS),Darwin)

# WARNS = -Wno-write-strings
WARNS = -Wno-write-strings -Wall -Wno-unused-variable -Wno-unused-but-set-variable -Wno-deprecated-declarations
ifeq ($(USE_SDL),1)
CPPFLAGS=$(SDL_INC)
LDFLAGS=$(SDL_LIBS)
else
CPPFLAGS=-I/opt/X11/include
LDFLAGS=-L/usr/X11/lib -lm -lX11
endif

all: heather

heather.o: heather.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heather.cpp $(WARNS) $(CPPFLAGS)

heathmsc.o: heathmsc.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathmsc.cpp $(WARNS) $(CPPFLAGS)

heathui.o: heathui.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathui.cpp $(WARNS) $(CPPFLAGS)

heathgps.o: heathgps.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathgps.cpp $(WARNS) $(CPPFLAGS)

heather: heather.o heathmsc.o heathui.o heathgps.o $(SDL_OBJS)
		  $(CC) heather.o heathui.o heathgps.o heathmsc.o $(SDL_OBJS) -o heather $(LDFLAGS)

SDL_gfxPrimitives.o: SDL_gfxPrimitives.cpp SDL_gfxPrimitives.h  makefile
		  $(CC) -c SDL_gfxPrimitives.cpp $(WARNS) $(CPPFLAGS)

heather_sdl.o: heather_sdl.cpp heather.ch makefile
		  $(CC) -c heather_sdl.cpp $(WARNS) $(CPPFLAGS)

clean:
		  rm heather.o heathui.o heathgps.o heathmsc.o $(SDL_OBJS) heather

endif



#
#  FreeBSD build
#

ifeq ($(OS),FreeBSD)
CC = cc
WARNS = -Wall -Wno-c++11-compat-deprecated-writable-strings
INCLUDES = -I /usr/local/include
LIBS = -L/usr/local/lib -lm -lX11

.SUFFIXES:
.SUFFIXES: .cpp .o

.cpp.o:
	$(CC) $(WARNS) $(INCLUDES) -c $< -o $@.new
	mv $@.new $@

SRCS = heather.cpp heathmsc.cpp heathui.cpp heathgps.cpp
OBJS = $(SRCS:cpp=o)

all: heather

heather: $(OBJS)
	$(CC) -o $@.new $(OBJS) $(LIBS)
	mv $@.new $@

$(OBJS): heather.ch heathfnt.ch makefile

clean:
	rm -f heather heather.new $(OBJS) $(OBJS:=.new)
endif
