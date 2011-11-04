set dir=/tmp
set mouse=a
set printoptions=bottom:10pc
set updatetime=1000000
set sessionoptions+=winpos
set backspace=2
set ruler

colorscheme torte
hi Search guibg=yellow

filetype on
filetype plugin on
filetype indent on
set linebreak
set showmatch
autocmd BufEnter * :syntax sync fromstart
"syn on
"highlight Folded guibg=bg guifg=#333333
"highlight FoldColumn guibg=#333333 guifg=fg

"spell right-clicking
set mousemodel=popup

"mac osx compatibility
set nocompatible
set backspace=indent,eol,start

let Tlist_Ctags_Cmd="~/exuberant/ctags-5.5.4/ctags"
map <F2> :Tlist
map <F3> :set guioptions+=m
map <F4> :sp:bn:set foldlevel=1:set foldlevel=1
"this refreshes folding & syntax
"map <F3> :set foldtext=getline(v:foldstart).'...'.getline(v:foldend)

map <F9>  :set ft=cpp
map <F10> :set ft=jtext
map <F11> :set ft=tex
map <F12> :syntax sync fromstart

map <S-Up> :up:bp
map <S-Down> :up:bn

map <C-Up> <C-U>
map <C-Down> <C-D>

map <C-X> :wa
map <C-D> :xa

"windowing
map <silent> <M-Up> :wincmd k
map <silent> <M-Down> :wincmd j
map <silent> <M-Left> :resize -3
map <silent> <M-Right> :resize +3

"toggle paren highlighting
let loaded_matchparen = 1
map <silent> ,q :NoMatchParen
map <silent> ,p :DoMatchParen

"spell checking
"map <silent> ,s :set spell
"map <silent> ,S :set nospell

map ,s ciw
map ,y yiw

map ,m :w:!make

map  <silent> ,/ :,s/$/--------------------------------------------------------------------------------:,s/.\{,78\}\zs.*//:/teh

"printing under KDE
set printexpr=PrintFile(v:fname_in)
function PrintFile(fname)
  call system("kprinter " . a:fname)
  call delete(a:fname)
  return v:shell_error
endfunc

"stop annoying html indenting
autocmd BufEnter *.html setlocal indentexpr= 
autocmd BufEnter *.htm setlocal indentexpr=
autocmd BufEnter *.xml setlocal indentexpr= 
autocmd BufEnter *.xsd setlocal indentexpr=

