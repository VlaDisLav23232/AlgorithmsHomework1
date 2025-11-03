CXX = g++

CXXFLAGS = -std=c++17 -O3 -Wall -Wextra

SOURCES = demo.cpp students.cpp
OBJECTS = $(SOURCES:.cpp=.o)
HEADERS = students.h

TARGET = benchmark
LIBS =
RM = rm -f

ifeq ($(OS),Windows_NT)
    TARGET = benchmark.exe
    RM = del /Q /F
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "Build finished: $(TARGET)"

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Clearing..."
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
	@echo "Done"

.PHONY: all clean