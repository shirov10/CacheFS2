all: Cache.o CacheFS.o CacheFS.a

Cache.o: Cache.h Cache.cpp
	g++ -std=c++11 -Wall -Wextra -c Cache.cpp -o Cache.o

CacheFS.o: CacheFS.h CacheFS.cpp Cache.o Cache.h
	g++ -std=c++11 -Wall -Wextra -c CacheFS.cpp -o CacheFS.o

CacheFS.a: CacheFS.o Cache.o
	ar -rcs CacheFS.a CacheFS.o Cache.o

clean:
	rm Cache.o CacheFS.o CacheFS.a