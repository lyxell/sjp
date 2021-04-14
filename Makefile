.PHONY: run

all: sjp
	@mkdir -p build
	./sjp

sjp: parser.cpp sjp.hpp
	$(CXX) -std=c++17 -D__EMBEDDED_SOUFFLE__ main.cpp parser.cpp -o $@

run: build/root.csv
	@cat build/root.csv

pretty_print: build/root.csv
	@cat build/root.csv | prettier --parser babel --trailing-comma none

build/root.csv: build/token.facts parser.dl
	souffle --fact-dir=build --output-dir=build parser.dl

build/token.facts: build/scanner Example.java
	build/scanner Example.java build/token.facts

build/scanner: build/scanner_re2c.c
	@mkdir -p build
	gcc -Wall -O2 -g -std=c99 build/scanner_re2c.c -o $@

build/%_re2c.c: %.c
	@mkdir -p build
	re2c -W --input-encoding utf8 -i $< -o $@

.PHONY: clean

clean:
	rm -rf build

parser.cpp: parser.dl
	souffle --no-warn \
			--generate=$@ \
			--fact-dir=build \
			--output-dir=build \
			parser.dl

sjp.hpp: sjp_re2c.hpp
	re2c -W --input-encoding utf8 -i $< -o $@
