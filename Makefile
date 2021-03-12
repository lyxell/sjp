build/scanner: build/scanner_re2c.c
	gcc build/scanner_re2c.c -o $@

build/%_re2c.c: %.c
	re2c -W --input-encoding utf8 -i $< -o $@
