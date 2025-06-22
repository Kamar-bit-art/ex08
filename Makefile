CXX = g++
OPTIMIZATION_LEVEL = -O2 -g
CXXFLAGS ?= -Wextra -Wall -pedantic -std=c++20 ${OPTIMIZATION_LEVEL} -fsanitize=undefined  -fsanitize=address
HEADERS = $(wildcard *.hpp *.h)
MAINS = $(basename $(wildcard *_main.cpp))
OBJECTS = $(addsuffix .o, $(filter-out $(MAINS), $(basename $(wildcard *.cpp))))

START_SEED = ${BUILD_GROUP}
ifndef BUILD_GROUP
START_SEED=42
endif

all: compile

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

%_main: %_main.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

compile: $(MAINS)

fuzzer: fuzzer_main
	echo "starting with seed ${START_SEED}"
	./fuzzer_main ${START_SEED} 1000
clean:
	rm -f *_main *.o

test: fuzzer_main
	./fuzzer_main
format:
	clang-format -i *.cpp *.hpp

.PHONY: clean fuzzer
