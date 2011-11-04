set autoindent
set tabstop=2
set shiftwidth=2
set smarttab
set expandtab
set softtabstop=2
set autoindent

set foldmethod=indent
set foldnestmax=4
set foldcolumn=5
set columns=85
highlight Folded guibg=bg guifg=#333333
highlight FoldColumn guibg=#333333 guifg=#777777
set fillchars="vert:|.fold:\ "
set foldtext=getline(v:foldstart).'...'

