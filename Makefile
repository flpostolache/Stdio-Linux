build: so-stdio.o
	gcc -shared so-stdio.o -o libso_stdio.so
so-stdio.o: so-stdio.c
	gcc -fPIC -c so-stdio.c
clean:
	rm -rf *.o *.so
