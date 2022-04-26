ergod:
	@gcc -o ergod ergod.c -Wall -pedantic `pkg-config --cflags --libs libsystemd`

all:
	ergod
