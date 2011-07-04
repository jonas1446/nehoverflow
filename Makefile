CC = g++
CFLAGS = -Wall
#LIBS = -L/usr/X11R6/lib -lGL -lglut -lGLU -lm -lnoise -L/usr/lib -lSDL -lpthread -I/usr/include/SDL -I/usr/local/include -L/usr/local/lib
LIBS = -lGL -lglut -lGLU -lm -lSDL -lpthread -lnoise -I/usr/include -L/usr/lib
OBJS = main.o timer.o camera.o map.o matrix4x4f.o vector3f.o noiseutils.o

all: t 

t: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^

