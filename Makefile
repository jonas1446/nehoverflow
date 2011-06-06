CC = g++
CFLAGS = -Wall
#LIBS = -L/usr/X11R6/lib -lGL -lglut -lGLU -lm -L/usr/lib -lSDL -lpthread -I/usr/include/SDL
LIBS = -lGL -lglut -lGLU -lm -lSDL -lpthread
OBJS = t.o

all: nehoverflow

nehoverflow: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^

