
all: default.dvi typed.dvi

default.dvi: default.jcode
	jtext2latex -C article default.jcode
typed.dvi: typed.jcode
	jtext2latex -C article typed.jcode

clean:
	rm -f notcon.* *.struct.* *.dvi *.pdf
cleaner: clean
	rm -f *.jdb *.jcode* *.jsess *.bz2

