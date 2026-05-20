# Makefile для ЛР4 — Автопарк (C++ + SQLite)

CXX = g++

CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -g

LDFLAGS = -lsqlite3

SRC = $(wildcard src/*.cpp)

OBJ = $(SRC:src/%.cpp=build/%.o)

TARGET = bin/autopark


all: $(TARGET)


$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@


check:
	@echo "=== Running make check ==="
	$(MAKE) all
	@echo "Build successful ✓"


distcheck:
	@echo "=== Running make distcheck ==="
	$(MAKE) clean
	$(MAKE) all
	@echo "Distcheck passed ✓"


clean:
	rm -rf bin/ build/ *.o *.db


.PHONY: all check distcheck clean
