CC = gcc
CFLAGS = -Wall -ansi
LIBS = -lglut -lGLU -lGL -lm
OBJS = main.o matrizes.o formas.o predio.o

main: $(OBJS)
	$(CC) -o main $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f main *.o
