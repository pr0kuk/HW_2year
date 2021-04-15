OBJECTS = $(SOURCES:.c=.o)

.PHONY: all $(OBJECTS)
all:
	gcc -c m16.c -o m16.o -g -O0 -fPIC
	gcc -c mn.c -o mn.o -g -O0 -fPIC
	gcc -c field.c -o field.o -g -O0 -fPIC
	gcc -shared m16.o -o m16.so -fPIC
	gcc -shared mn.o -o mn.so -fPIC
	gcc -shared field.o -o libflib.so
	gcc main.c -o main -g -O0 -L $(shell pwd) -lflib -ldl

.PHONY: clean
clean:
	-rm *.o
	-rm main
	-rm *.so
