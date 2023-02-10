CXX = clang++-15
CC = clang-15

CXXFLAGS = -std=c++20 -O3 -g3 --debug -Wall -Wextra
CXXFLAGS += -I/usr/include/dpp-10.0/dpp/
CXXFLAGS += -I./libs/boost_1_81_0/boost/
CXXFLAGS += -I./bin/pch/

CXXLIBS += -ldpp
CXXLIBS += -lpqxx
CXXLIBS += -L/usr/lib/dpp-10.0/

CXXSRC = $(wildcard src/*.cxx)
CXXSRC += $(wildcard src/sql/*.cxx)
CXXSRC += $(wildcard src/impl/*.cxx)

BIN = bin
OUTPUT = program
BINOBJ = $(CXXSRC:src/%.cxx=$(BIN)/obj/%.o)

all: dirs program

run: all
	$(BIN)/$(OUTPUT)

pch: dirs
	$(CC) -xc++-header src/pch.hpp -o $(BIN)/pch/pch.pch $(CXXFLAGS)

program: $(BINOBJ)
	$(CXX) $(BINOBJ) -o $(BIN)/$(OUTPUT) $(CXXLIBS)

$(BIN)/obj/%.o: src/%.cxx $(BIN)/pch/pch.pch
	$(CXX) $< -include-pch $(BIN)/pch/pch.pch -c -o $@ $(CXXFLAGS)

dirs:
	mkdir -p $(BIN)/obj $(BIN)/pch $(BIN)/obj/sql $(BIN)/obj/impl

cleanobjs:
	rm -rf $(BIN)/obj

clean:
	rm -rf $(BIN)
