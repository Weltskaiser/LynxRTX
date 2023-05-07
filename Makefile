# Release
CXX_PROFILE = -O3
# Debug
# CXX_PROFILE = -g
# Sanitize
# CXX_PROFILE = -g -fsanitize=address

CXXFLAGS = -std=c++20 -Wall -Wextra $(CXX_PROFILE) -I src

SRC = $(wildcard ./*.cpp)
OBJ = $(SRC:.cpp=.o)
LIB = -lsfml-system -lsfml-graphics -lsfml-window

TARGET = LYNX_RTX

all: $(TARGET)
clean:
	rm -f $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LIB) -o $(TARGET)

.PHONY: all clean