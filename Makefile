CXX = g++

CXXFLAGS = -std=c++17 -O3 -Wall -Wextra

SOURCES = demo.cpp students.cpp
OBJECTS = $(SOURCES:.cpp=.o)
HEADERS = students.h

TARGET = demo
LIBS =
RM = rm -f

ifeq ($(OS),Windows_NT)
	TARGET = demo.exe
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
	$(RM) results.csv
	$(RM) students_100.csv
	$(RM) students_1000.csv
	$(RM) students_10000.csv
	$(RM) students_100000.csv
	$(RM) demo_sorted_standard.csv
	$(RM) demo_sorted_radix.csv
	$(RM) sorted_standard_hash.csv
	$(RM) sorted_standard_bst.csv
	$(RM) sorted_standard_vector.csv
	$(RM) sorted_students_radix.csv
	$(RM) sorted_students.csv
	$(RM) result.png
	$(RM) plot_results.py
	$(RM) sorted_students_standard.csv
	@echo "Done"

.PHONY: all clean