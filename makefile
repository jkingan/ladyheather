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

ifeq ($(OS),Linux)

# WARNS = -Wno-write-strings 
WARNS = -Wno-write-strings -Wall -Wno-unused-but-set-variable -Wno-unused-variable 

all: heather

heather.o: heather.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heather.cpp $(WARNS)

heathmsc.o: heathmsc.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathmsc.cpp $(WARNS)

heathui.o: heathui.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathui.cpp $(WARNS)

heathgps.o: heathgps.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathgps.cpp $(WARNS)

heather: heather.o heathmsc.o heathui.o heathgps.o
		  $(CC) heather.o heathui.o heathgps.o heathmsc.o -o heather -lm -lX11

clean:
		  rm heather.o heathui.o heathgps.o heathmsc.o heather

endif


#
# OS/X build
# 

ifeq ($(OS),Darwin)

# WARNS = -Wno-write-strings 
WARNS = -Wno-write-strings -Wall -Wno-unused-variable 

all: heather

heather.o: heather.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heather.cpp $(WARNS) -I/opt/X11/include

heathmsc.o: heathmsc.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathmsc.cpp $(WARNS) -I/opt/X11/include 

heathui.o: heathui.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathui.cpp $(WARNS) -I/opt/X11/include 

heathgps.o: heathgps.cpp heather.ch heathfnt.ch makefile
		  $(CC) -c heathgps.cpp $(WARNS) -I/opt/X11/include 

heather: heather.o heathmsc.o heathui.o heathgps.o
		  $(CC) heather.o heathui.o heathgps.o heathmsc.o -o heather -L/usr/X11/lib -lm -lX11

clean:
		  rm heather.o heathui.o heathgps.o heathmsc.o heather

endif


