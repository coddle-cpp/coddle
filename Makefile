TARGET=$(shell basename $$(pwd))
SOURCES=$(shell echo *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
CXXFLAGS=-Wall -march=native -gdwarf-3 -std=c++1y -O3 -g -pthread
all: $(TARGET) tests/library_test/library_test tests/simple_test/simple_test tests/subproject_test/bin/bin
$(TARGET): $(OBJECTS)
	clang++ $(OBJECTS) -pthread -ldl -o $(TARGET)
%.o: %.cpp
	clang++ $(CXXFLAGS) -c $< -MT $@ -MMD -MP -MF $*.mk~
-include *.mk~
clean:
	rm -f *~
	rm -f *.o
	rm -f $(TARGET)
tests/simple_test/simple_test: coddle FORCE
	echo "coddle: Entering directory \`tests/simple_test'";\
	cd tests/simple_test && rm -rf .coddle simple_test && ../../coddle;\
	echo "coddle: Leaving directory \`tests/simple_test'"
tests/library_test/library_test: coddle FORCE
	echo "coddle: Entering directory \`tests/library_test'";\
	cd tests/library_test && rm -rf .coddle library_test && ../../coddle && ../../coddle;\
	echo "coddle: Leaving directory \`tests/library_test'"
tests/subproject_test/bin/bin: coddle FORCE
	echo "coddle: Entering directory \`tests/subproject_test'";\
	cd tests/subproject_test && rm -rf .coddle bin/bin library/liblibrary.a && ../../coddle;\
	echo "coddle: Leaving directory \`tests/subproject_test'"
FORCE:
