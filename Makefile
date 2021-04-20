.PHONY: run

CXXFLAGS=-std=c++17 -O2 -D__EMBEDDED_SOUFFLE__

all: sjp.o parser.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

parser.cpp: parser.dl
	souffle --no-warn \
			--generate=$@ \
			--fact-dir=build \
			--output-dir=build \
			parser.dl

sjp.cpp: sjp_re2c.cpp
	re2c -W --input-encoding utf8 -i $< -o $@

example: sjp.o parser.o
	$(CXX) $(CXXFLAGS) sjp.o parser.o main.cpp -o $@

.PHONY: clean

clean:
	rm -rf sjp.o parser.o example sjp.cpp parser.cpp
