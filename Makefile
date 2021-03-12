.PHONY: run

run: statement.csv
	cat statement.csv

statement.csv: token.facts parser.dl
	souffle parser.dl

token.facts: build/scanner
	build/scanner > token.facts

build/scanner: build/scanner_re2c.c
	gcc -O2 -std=c99 build/scanner_re2c.c -o $@

build/%_re2c.c: %.c
	re2c -W --input-encoding utf8 -i $< -o $@
