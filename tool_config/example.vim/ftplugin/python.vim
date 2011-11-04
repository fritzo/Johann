set nocindent
set smartindent
set cinwords=if,elif,else,for,while,try,except,finally,def,class
set tabstop=2
set shiftwidth=2
set smarttab
set expandtab
set softtabstop=2
set autoindent

set foldmethod=indent
set foldnestmax=2
set foldcolumn=3
set columns=83
highlight Folded guibg=bg guifg=#333333
highlight FoldColumn guibg=#333333 guifg=#777777
set fillchars="vert:|.fold:\ "
set foldtext=getline(v:foldstart).'...'
set foldignore=

let Tlist_Exit_OnlyWindow = 1

