TARGET = oss userP
all: main userP
main: main.c sharedMem.h queue.h
	gcc -o oss main.c
user: userP.c sharedMem.h
	gcc -o userP userP.c
clean:
	/bin/rm -f *.o $(TARGET)

