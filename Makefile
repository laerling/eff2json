.PHONY: build run debug test clean

cc=gcc -std=c99 -Wpedantic
# $(wildcard src/**/*) does not work with files directly under src/ for some reason
source_files=$(filter-out src/test.c, $(shell find src/ -type f -name '*.[ch]'))
source_files_test=$(filter-out src/main.c, $(shell find src/ -type f -name '*.[ch]'))


build: build/eff2json

run: build/eff2json
	# giving the executable itself as argument
	$< $<

debug: build/eff2json_debug
	gdb $<

test: build/eff2json_test
	$<

clean:
	rm build/*


build/eff2json: $(source_files)
	mkdir -p build
	$(cc)  -o $@  $+

build/eff2json_debug: $(source_files)
	mkdir -p build
	$(cc)  -o $@  -ggdb -g3  $+

build/eff2json_test: $(source_files_test)
	mkdir -p build
	$(cc)  -o $@  -ggdb -g3  -DEFF2JSON_TEST  $+
