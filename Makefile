ifndef CXX
CXX := c++
endif

ifndef LIBNL_INCLUDE_PATH
LIBNL_INCLUDE_PATH := -I/usr/include/libnl3
endif

CXXFLAGS := $(CXXFLAGS) -std=c++20 -pedantic -Wall -Wextra $(LIBNL_INCLUDE_PATH)
LDLIBS := $(LDLIBS) -lnl-3

.PHONY: all clean

all: client server

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp $(LDLIBS)

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp $(LDLIBS)

clean:
	-rm -f client server
