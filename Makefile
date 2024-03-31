all: sparse-gen

sparse-gen: sparse-gen.o
	    gcc -o sparse-gen sparse-gen.o

sparse-gen.o: sparse-gen.c
	      gcc -c sparse-gen.c

.PHONY: all