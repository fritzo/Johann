
all: install

build:
	mkdir build

install: build
	(cd build && cmake .. && make && make install)

test: build
	(cd build && cmake .. && make && echo '' > test.log && \
	POMAGMA_LOG_LEVEL=3 POMAGMA_LOG_FILE=../test.log make test || \
	(grep -C3 -i error test.log && false))

clean: FORCE
	rm -rf lib build
	find . -type f | grep '\.log$$' | xargs rm -f
	find . -type f | grep '^core' | xargs rm -f

FORCE:
