
CC = g++

# libraries required to compile opengl/glut programs may be system-dependent
# this is a setup for a typical install of fedora 3-7

LIBOPT = -L/usr/X11R6/lib -lglut -lGLU -lGL -lm

all : proj4

proj4 : proj4.o
	$(CC) -o proj4 proj4.o $(LIBOPT)

proj4.o : proj4.cpp Makefile
	$(CC) -I/usr/X11R6/include -c proj4.cpp

clean : 
	rm *.o proj4

