CC = g++
CFLAGS = -Wall
#LIBS = -L/usr/X11R6/lib -lGL -lglut -lGLU -lm -L/usr/lib -lSDL -lpthread -I/usr/include/SDL
LIBS = -lGL -lglut -lGLU -lm -lSDL -lpthread
OBJS = main.o timer.o camera.o map.o matrix4x4f.o vector3f.o 

all: t 

t: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^

