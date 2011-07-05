CC = g++
CFLAGS = -Wall
#LIBS = -L/usr/X11R6/lib -lGL -lglut -lGLU -lm -lnoise -L/usr/lib -lSDL -lpthread -I/usr/include/SDL -I/usr/local/include -L/usr/local/lib
LIBS = -lGL -lglut -lGLU -lm -lSDL -lpthread 
OBJS = main.o timer.o camera.o map.o matrix4x4f.o vector3f.o lib/glm.o lib/imageloader.o lib/Texture.o

all: nehoverflow

nehoverflow: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^

