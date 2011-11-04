"set guifont=MiscFixed\ 10
"set guifont=6x10
set guioptions=airL
set guiheadroom=0
set lines=96 columns=81

"mac osx compatibility
set nocompatible
set backspace=indent,eol,start

"font size changing (ubuntu + proggy fonts)
set guifont=ProggyTinyTT\ 12
map <F5> :set guifont=ProggyTinyTT\ 12
map <F6> :set guifont=ProggySmallTT\ 12
map <F7> :set guifont=ProggyCleanTT\ 12
map <F8> :set guifont=DejaVu\ Sans\ Mono\ 12

"font size changing (gentoo)
"set guifont=Fixed\ 7,Monospace\ 6,\ 6x10
"map <F5> :set guifont=Fixed\ 5
"map <F6> :set guifont=Fixed\ 7,Monospace\ 6,\ 6x10
"map <F7> :set guifont=Fixed\ 10
"map <F8> :set guifont=Fixed\ 14

"font size changing (xp+cygwin+X11)
"set guifont=Fixed\ Semi-Condensed\ 10
"map <F5> :set guifont=Fixed\ Semi-Condensed\ 10
"map <F6> :set guifont=Fixed\ Semi-Condensed\ 12
"map <F7> :set guifont=Fixed\ Semi-Condensed\ 14
"map <F8> :set guifont=Fixed\ Semi-Condensed\ 20

"font size changing for both gtk1.2 and gtk2
"set guifont=Terminus\ 6,Monospace\ 6,Fixed\ 7,\ 6x10
"map <F5> :set guifont=Monospace\ 5,Fixed\ 5,\ 5x8
"map <F6> :set guifont=Terminus\ 6,Monospace\ 6,Fixed\ 7,\ 6x10
"map <F7> :set guifont=Monospace\ 8,Fixed\ 10,\ 7x13
"map <F8> :set guifont=Monospace\ 12,Fixed\ 14,\ 10x20

"syntax on
"syntax enable
"if exists("syntax_on") | | else | syntax on | endif
set hlsearch
highlight Normal guibg=white

"Light background
"highlight Normal guibg=white

"Dark background
colorscheme torte
highlight Normal guifg=gray

