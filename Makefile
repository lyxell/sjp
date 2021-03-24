.PHONY: run

run: build/class_declaration.csv
	@cat build/class_declaration.csv

build/class_declaration.csv: build/token.facts parser.dl
	souffle --fact-dir=build --output-dir=build --no-warn parser.dl

build/token.facts: build/scanner Example.java
	build/scanner Example.java > build/token.facts

build/scanner: build/scanner_re2c.c
	@mkdir -p build
	gcc -Wall -O2 -g -std=c99 build/scanner_re2c.c -o $@

build/%_re2c.c: %.c
	@mkdir -p build
	re2c -W --input-encoding utf8 -i $< -o $@

.PHONY: clean

clean:
	rm -rf build
