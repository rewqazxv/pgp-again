test: test.cpp aes.o rsa.o
	g++ -o test -lgmpxx -lgmp test.cpp aes.o rsa.o

aes.o: ../encrypt/aes.c ../encrypt/aes.h
	gcc -c ../encrypt/aes.c

rsa.o: ../encrypt/rsa.cpp ../encrypt/rsa.hpp
	g++ -c ../encrypt/rsa.cpp

run: test
	./test

clean:
	rm -rf *.o test
