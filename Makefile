.PHONY: debug run clean

build/eff2json: src/*.c src/*.h
	mkdir -p build
	gcc -o $@ $+

build/eff2json_debug: src/*.c src/*.h
	mkdir -p build
	gcc -g3 -o $@ $+ && gdb $@

debug: build/eff2json_debug

run: build/eff2json
	# giving the executable itself as argument
	$< $<

clean:
	rm build/*
