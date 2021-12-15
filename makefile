CC        := icx
STD       := -std=c++17
TBB       := -I $(TBBROOT)/include -L $(TBBROOT)/lib/intel64/gcc4.8
CFLAGS    := -c -Wall -O2 -xCore-AVX512 -qopt-zmm-usage=high
LIBRARIES := $(TBB) -ltbb -lstdc++

.PHONY: all clean

all: app

app: main.o
	$(CC) $(STD) main.o $(LIBRARIES) -o app

main.o: main.cpp
	$(CC) $(STD) $(CFLAGS) main.cpp $(LIBRARIES)

clean:
	rm -rf app *.o
