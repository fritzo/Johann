
all: install

build:
	mkdir build

log:
	mkdir log

install: build
	(cd build && cmake .. && $(MAKE) && $(MAKE) install)

test: build log
	echo 'PWD =' `pwd`
	(cd build && cmake ..)
	$(MAKE) -C build
	echo '' > log/test.log
	POMAGMA_LOG_LEVEL=3 POMAGMA_LOG_FILE=log/test.log $(MAKE) -C build test \
	|| (grep -C3 -i error log/test.log && false)

clean: FORCE
	rm -rf lib build log
	find . -type f | grep '\.log$$' | xargs rm -f
	find . -type f | grep '^core' | xargs rm -f

FORCE:
