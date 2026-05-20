# Makefile для ЛР4 — Автопарк (C + SQLite)

CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -Iautopark_app/include -g

LDFLAGS = -lsqlite3

SRC = $(wildcard autopark_app/src/*.c)

OBJ = $(SRC:autopark_app/src/%.c=build/%.o)

TARGET = bin/autopark


all: $(TARGET)


$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


build/%.o: autopark_app/src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@


# === Обязательные цели для GitHub Actions ===

check:
	@echo "=== Running make check ==="
	$(MAKE) all
	@echo "Build successful ✓"


distcheck:
	@echo "=== Running make distcheck ==="
	$(MAKE) clean
	$(MAKE) all
	@echo "Distcheck passed - project builds cleanly ✓"


clean:
	rm -rf bin/ build/ *.o *.db


.PHONY: all check distcheck clean
