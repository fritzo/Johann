
all: html
	$(MAKE) safe

html: FORCE
	$(MAKE) -C doc all

safe: FORCE
	chmod -f 600 makefile
	chmod -f 644 *.html *.js *.css *.ico
	chmod -f 711 doc doxygen
	$(MAKE) -C doc safe

clean:
	$(MAKE) -C doc clean
	rm -f doxygen/* *.pdf

FORCE:





