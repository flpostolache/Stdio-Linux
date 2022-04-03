build: so-stdio.o
	gcc -shared $^ -o libso_stdio.so
so-stdio.o: so-stdio.c
	gcc -fPIC -c $^ -o $@
clean:
	rm -rf *.o *.so
