EXE = example
OBJS = sjp.o parser.o

CXXFLAGS=-std=c++17 -fPIC -fno-gnu-unique -O2 -D__EMBEDDED_SOUFFLE__

all: $(OBJS)

parser.cpp: parser.dl
	souffle --no-warn \
			--generate=$@ \
			--fact-dir=build \
			--output-dir=build \
			parser.dl

sjp.cpp: sjp_re2c.cpp
	re2c -W --input-encoding utf8 -i $< -o $@

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(EXE).cpp -o $@

.PHONY: clean

clean:
	rm -rf $(OBJS) $(EXE) sjp.cpp parser.cpp
