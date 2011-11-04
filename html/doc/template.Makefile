
all: 

safe: FORCE
	chmod -f 600 makefile
	chmod -f 644 *.html || true
	chmod -f 644 *.text || true
	chmod -f 644 *.jtext || true
	chmod -f 644 *.jcode || true

clean:
	rm -f *.html *.text *.jtext *.jcode

FORCE:

