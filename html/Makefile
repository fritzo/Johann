
all: html
	$(MAKE) safe

html: FORCE
	$(MAKE) -C doc all

safe: FORCE
	chmod -f 600 makefile
	chmod -f 644 *.html *.js *.css *.ico
	$(MAKE) -C doc safe

clean:
	$(MAKE) -C doc clean

FORCE:





