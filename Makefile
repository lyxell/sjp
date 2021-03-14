.PHONY: run

run: class_declaration.csv
	cat class_declaration.csv

class_declaration.csv: token.facts parser.dl
	souffle --no-warn parser.dl

token.facts: build/scanner
	build/scanner > token.facts

build/scanner: build/scanner_re2c.c
	gcc -Wall -O2 -g -std=c99 build/scanner_re2c.c -o $@

build/%_re2c.c: %.c
	re2c -W --input-encoding utf8 -i $< -o $@
