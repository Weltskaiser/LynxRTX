# CXX = clang++

CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -I src

SRC = $(wildcard ./*.cpp)
OBJ = $(SRC:.cpp=.o)

TARGET = LYNX_RTX

all: $(TARGET)
clean:
	rm -f $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

.PHONY: all clean