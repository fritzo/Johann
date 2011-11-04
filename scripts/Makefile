
current_target: main.dvi

all: main.pdf html
pdf: main.pdf

.PHONEY: html clean FORCE

#specific to it
xdvi = xdvi -s 7 -geometry 825x970 -fg black -bg white -xrm "xdvi.wheelTranslations: <Btn5Down>: forward-page()\n<Btn4Down>: back-page()\n"

#dependency tree
depend: FORCE
	./depends . axioms sk skj skrj skjo examples user build test
	mv dependency.* html/

#latex documentation
figs: extensions.eps FORCE
	$(MAKE) -C skrj figs
	$(MAKE) -C algorithms figs
main.dvi: main.text figs FORCE
	jtext2latex -C memoir,fullpage -b -i main
main.pdf: main.dvi
	dvipdf main.dvi
view: main.dvi
	$(xdvi) main.dvi &

#nightly pdf posting via crontab
post: FORCE
	~/johann/jtext/jtext2latex -C memoir,fullpage -b -i main
	~/johann/jtext/jtext2latex -C memoir,fullpage -b -i main
	~/johann/jtext/jtext2latex -C memoir,fullpage -b -i main
	dvipdf main.dvi
	cp main.pdf fritz-thesis-draft.pdf
	scp fritz-thesis-draft.pdf fho@qwe2.math.cmu.edu:html/

abstract.dvi: abstract.text
	jtext2latex -C article,fullpage abstract
abstract.pdf: abstract.dvi
	dvipdf abstract.dvi

extensions.eps: extensions.fig
	fig2dev -L eps extensions.fig extensions.eps

html: depend FORCE
	python ../jtext/jtext2html -o html -j all.jtext -g -d -i . algorithms axioms sk skr skj skrj skjo examples user build test help

clean:
	rm -f *.log *.aux *.jtext.tex *.text.tex *.jcode.tex *.dvi *.pdf temp.diff *.toc *.bbl *.blg *.idx *.dlg  *.ilg *.ind *.out core core.* all.jtext
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


