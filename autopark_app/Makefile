CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iincludes
LDFLAGS := -lsqlite3

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp,build/%.o,$(SRC))
TARGET := bin/autopark

TEST_SRC := tests/test_autopark.cpp src/Auth.cpp src/AutoparkService.cpp src/Database.cpp
TEST_TARGET := bin/autopark_tests

COVERAGE_DIR := coverage
COVERAGE_FLAGS := --coverage -O0 -g

.PHONY: all run clean check test coverage distcheck rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin data
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)

$(TEST_TARGET): $(TEST_SRC)
	mkdir -p bin build
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_TARGET) $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

check: test

distcheck: clean all check

coverage:
	mkdir -p bin build $(COVERAGE_DIR)
	rm -f *.gcda *.gcno *.gcov src/*.gcda src/*.gcno tests/*.gcda tests/*.gcno $(COVERAGE_DIR)/*.gcov $(COVERAGE_DIR)/gcov-summary.txt
	$(CXX) $(CXXFLAGS) $(COVERAGE_FLAGS) $(TEST_SRC) -o $(TEST_TARGET) $(LDFLAGS)
	./$(TEST_TARGET)
	gcov -r -o bin/autopark_tests-test_autopark.gcno tests/test_autopark.cpp > $(COVERAGE_DIR)/gcov-summary.txt
	gcov -r -o bin/autopark_tests-Auth.gcno src/Auth.cpp >> $(COVERAGE_DIR)/gcov-summary.txt
	gcov -r -o bin/autopark_tests-AutoparkService.gcno src/AutoparkService.cpp >> $(COVERAGE_DIR)/gcov-summary.txt
	gcov -r -o bin/autopark_tests-Database.gcno src/Database.cpp >> $(COVERAGE_DIR)/gcov-summary.txt
	mv *.gcov $(COVERAGE_DIR)/ 2>/dev/null || true
	@echo "Coverage files are in $(COVERAGE_DIR)/"
	@grep -E "File 'tests/|File 'src/|Lines executed" $(COVERAGE_DIR)/gcov-summary.txt

rebuild: clean all

clean:
	rm -rf build bin/autopark bin/autopark_tests $(COVERAGE_DIR) *.gcda *.gcno *.gcov src/*.gcda src/*.gcno tests/*.gcda tests/*.gcno
