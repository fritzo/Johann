
current_target: main.dvi
all: main.pdf

main.dvi: main.text FORCE
	jtext2latex -C amsbook,fullpage -b main
main.pdf: main.dvi
	dvipdf main.dvi
view: main.dvi
	xdvi main.dvi &

clean:
	rm -f *.tex *.aux *.bbl *.blg *.log *.dvi *.pdf *.ps

FORCE:


