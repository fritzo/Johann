
all: bib html
	$(MAKE) safe

safe: FORCE
	chmod -f 600 makefile template.makefile
	chmod -f 644 *.html *.css
	chmod -f 644 *.jtext || true
	chmod -f 644 *.jcode || true
	chmod -f 644 *.text || true
	chmod -f 711 algorithms axioms sk skr skj skrj skjo examples user build test help
	$(MAKE) -C algorithms safe
	$(MAKE) -C axioms safe
	$(MAKE) -C sk safe
	$(MAKE) -C skr safe
	$(MAKE) -C skj safe
	$(MAKE) -C skrj safe
	$(MAKE) -C skjo safe
	$(MAKE) -C examples safe
	$(MAKE) -C user safe
	$(MAKE) -C build safe
	$(MAKE) -C test safe
	$(MAKE) -C help safe

#using http://www.lri.fr/~filliatr/bibtex2html/
bib: FORCE
	bibtex2html -o bib -css "bib.css" ../../extern/latex/fho_refs.bib

html: FORCE
	$(MAKE) -C ../../scripts html

clean:
	rm -f *.html *.text *.jtext *.jcode dependency.*
	$(MAKE) -C algorithms clean
	$(MAKE) -C axioms clean
	$(MAKE) -C sk clean
	$(MAKE) -C skr clean
	$(MAKE) -C skj clean
	$(MAKE) -C skrj clean
	$(MAKE) -C skjo clean
	$(MAKE) -C examples clean
	$(MAKE) -C user clean
	$(MAKE) -C build clean
	$(MAKE) -C test clean
	$(MAKE) -C help clean

FORCE:


