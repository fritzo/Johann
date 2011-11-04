
current_target: main.dvi

all: main.pdf
figs: distributivity.eps

main.dvi: main.text FORCE
	jtext2latex -C amsbook,fullpage -b main
main.pdf: main.dvi
	dvipdf main.dvi
view: main.dvi
	xdvi main.dvi &

distributivity.eps: distributivity.fig
	fig2dev -L eps distributivity.fig distributivity.eps

clean:
	rm -f *.tex *.aux *.bbl *.blg *.log *.dvi *.pdf *.ps

FORCE:


