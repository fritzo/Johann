
current_target: main.dvi
all: main.pdf

figs: SKJ_space.eps

SKJ_space.eps: SKJ_space.fig
	fig2dev -L eps SKJ_space.fig SKJ_space.eps

main.dvi: main.text FORCE
	jtext2latex -C amsbook,fullpage main
main.pdf: main.dvi
	dvipdf main.dvi
view: main.dvi
	xdvi -fg black main.dvi &

clean:
	rm -f *.log *.aux *.tex *.out *.dvi *.pdf *.eps temp.diff *.toc *.bbl *.blg

FORCE:

