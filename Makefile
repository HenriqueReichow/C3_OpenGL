CC = gcc
CFLAGS = -Wall -ansi
LIBS = -lglut -lGLU -lGL -lm
OBJS = main.o matrizes.o formas.o predio.o

main: $(OBJS)
	$(CC) -o main $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f main *.o C3_OpenGL.zip

# pacote de entrega: código, Makefile, foto de referência e o texto com os integrantes
zip:
	rm -f C3_OpenGL.zip
	zip C3_OpenGL.zip *.c *.h Makefile c3.webp LEIA-ME.txt
