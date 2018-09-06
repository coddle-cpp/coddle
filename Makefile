TARGET=$(shell basename $$(pwd))
SOURCES=$(shell echo *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
CXXFLAGS=-Wall -march=native -gdwarf-3 -std=c++17 -O0 -g -pthread -D_GLIBCXX_DEBUG
all: $(TARGET)
$(TARGET): $(OBJECTS)
	clang++ $(OBJECTS) -pthread -ldl -o $(TARGET)
%.o: %.cpp
	clang++ $(CXXFLAGS) -c $< -MT $@ -MMD -MP -MF $*.mk~
-include *.mk~
clean:
	rm -f *~
	rm -f *.o
	rm -f $(TARGET)
FORCE:
