test: test.cpp aes.o rsa.o
	g++ -o test -lgmpxx -lgmp test.cpp aes.o rsa.o

aes.o: ../src/encrypt/aes.c ../src/encrypt/aes.h
	gcc -c ../src/encrypt/aes.c

rsa.o: ../src/encrypt/rsa.cpp ../src/encrypt/rsa.hpp
	g++ -c ../src/encrypt/rsa.cpp

run: test
	./test

clean:
	rm -rf *.o test
