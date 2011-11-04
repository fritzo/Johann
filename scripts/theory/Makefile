
current_target: main.dvi
all: main.pdf

main.dvi: main.text FORCE
	jtext2latex -C amsbook,fullpage -b main
main.pdf: main.dvi
	dvipdf main.dvi
view: main.dvi
	xdvi -fg black main.dvi &

clean:
	rm -f *.tex *.log *.aux *.dvi *.pdf *.pyc *.pyo *.eps *.ps *.png temp.diff *.toc *.bbl *.blg

FORCE:

