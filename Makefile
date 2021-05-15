EXE = example
OBJS = lexer.o parser.o program.o

CXXFLAGS=-std=c++17 -O2 -Wfatal-errors -fPIC -march=native -fno-gnu-unique -D__EMBEDDED_SOUFFLE__

SOUFFLE=souffle
ifdef SOUFFLE_PATH
	SOUFFLE=$(SOUFFLE_PATH:%/=%)/src/souffle
	CXXFLAGS+=-I$(SOUFFLE_PATH:%/=%)/src/include
endif

all: $(OBJS)

program.o: program.dl
	$(SOUFFLE) --no-warn --generate=sjp $<
	echo '#include "souffle/profile/Logger.h"' > tmp.cpp
	echo '#include "souffle/profile/ProfileEvent.h"' >> tmp.cpp
	cat sjp.cpp >> tmp.cpp
	rm sjp.cpp
	$(CXX) $(CXXFLAGS) tmp.cpp -c -o $@
	rm tmp.cpp

lexer.o: lexer.cpp
	re2c -W --input-encoding utf8 -i $< -o lexer_generated.cpp
	$(CXX) $(CXXFLAGS) lexer_generated.cpp -c -o $@
	rm lexer_generated.cpp

parser.o: parser.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(EXE).cpp -o $@

.PHONY: clean

clean:
	rm -rf $(OBJS) $(EXE)
