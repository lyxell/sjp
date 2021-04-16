.PHONY: run

all: sjp
	@mkdir -p build
	./sjp

sjp: parser.cpp sjp.hpp main.cpp
	$(CXX) -std=c++17 -O2 -D__EMBEDDED_SOUFFLE__ main.cpp parser.cpp -o $@

pretty_print: build/root.csv
	@cat build/root.csv | prettier --parser babel --trailing-comma none

exec:
	cp build/token_type.csv build/token_type.facts
	cp build/token.csv build/token.facts
	cp build/num_tokens.csv build/num_tokens.facts
	souffle --no-warn --fact-dir=build --output-dir=build --profile=build/profile parser.dl
	cat build/in_tree.csv

profile:
	cp build/token_type.csv build/token_type.facts
	cp build/token.csv build/token.facts
	cp build/num_tokens.csv build/num_tokens.facts
	souffle --fact-dir=build --output-dir=build --profile=build/profile parser.dl
	souffle-profile build/profile -j

parser.cpp: parser.dl
	souffle --no-warn \
			--generate=$@ \
			--fact-dir=build \
			--output-dir=build \
			parser.dl

sjp.hpp: sjp_re2c.hpp
	re2c -W --input-encoding utf8 -i $< -o $@

.PHONY: clean

clean:
	rm -rf build
