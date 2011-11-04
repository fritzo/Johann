" my filetype file
if exists("did_load_filetypes")
  finish
endif
augroup filetypedetect
  au! BufRead,BufNewFile *.text		setfiletype jtext
  au! BufRead,BufNewFile *.jtext	setfiletype jtext
  au! BufRead,BufNewFile *.jcode	setfiletype jcode
  au! BufRead,BufNewFile *.c		setfiletype cpp
  au! BufRead,BufNewFile *.y		setfiletype yacc
  au! BufRead,BufNewFile *.output	setfiletype output
  au! BufRead,BufNewFile *.csv		setfiletype csv
augroup END

"catch recursion in an if statement HACK
if !exists("syntax_on") | syntax on | endif

