EXE = example
OBJS = sjp.o parser.o

CXXFLAGS=-std=c++17 -O2 -fPIC -march=native -fno-gnu-unique -D__EMBEDDED_SOUFFLE__

SOUFFLE=souffle
ifdef SOUFFLE_PATH
	SOUFFLE=$(SOUFFLE_PATH:%/=%)/src/souffle
	CXXFLAGS+=-I$(SOUFFLE_PATH:%/=%)/src/include
endif

all: $(OBJS)

parser.cpp: parser.dl
	$(SOUFFLE) --no-warn \
			--generate=$@ \
			--fact-dir=build \
			--output-dir=build \
			parser.dl
	echo '#include "souffle/profile/Logger.h"' > tmp.cpp
	echo '#include "souffle/profile/ProfileEvent.h"' >> tmp.cpp
	cat $@ >> tmp.cpp
	mv tmp.cpp $@

sjp.cpp: sjp_re2c.cpp
	re2c -W --input-encoding utf8 -i $< -o $@

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(EXE).cpp -o $@

.PHONY: clean

clean:
	rm -rf $(OBJS) $(EXE) sjp.cpp parser.cpp
