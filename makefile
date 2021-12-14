CC        := g++
STD       := -std=c++17
TBB       := -I /home/owerbat/intel/oneapi/tbb/latest/include -L /home/owerbat/intel/oneapi/tbb/latest/lib/intel64/gcc4.8
CFLAGS    := -c -Wall -O2
LIBRARIES := $(TBB) -ltbb

.PHONY: all clean

all: app

app: main.o
	$(CC) $(STD) main.o $(LIBRARIES) -o app

main.o: main.cpp
	$(CC) $(STD) $(CFLAGS) main.cpp $(LIBRARIES)

clean:
	rm -rf app *.o
