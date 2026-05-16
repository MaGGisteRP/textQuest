# Fallback build without CMake.
#   make        -> build ./novel
#   make run    -> build and run
#   make clean  -> remove artifacts

CXX      ?= g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude
SRC      := src/main.cpp src/Game.cpp src/States.cpp scenes/Story.cpp
TARGET   := novel

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf saves

.PHONY: run clean
