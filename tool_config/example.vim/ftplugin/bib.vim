"indenting
set autoindent
set tabstop=2
set shiftwidth=2
set shiftround
set smarttab
set expandtab
set softtabstop=2

"folding
set foldmethod=expr
set foldexpr=getline(v:lnum)[0]!=\"@\"
set foldnestmax=1
set foldcolumn=2
highlight Folded guibg=bg guifg=#333333
highlight FoldColumn guibg=#333333 guifg=#777777
set fillchars="vert:|.fold:\ "
set foldtext=getline(v:foldstart).'...'

highlight bibEntry guifg=black guibg=white gui=bold
syntax match NoComma ".\+\ze[^,]$" contained containedin=ALL
highlight NoComma guifg=white guibg=red

"environment shortcuts
map ,q :%s///ge:%s/=[ ]*/= /ge:%s/"\([^"]*\)",/{\1},/e:%s/^[ ]\+/  /e:%s/\(topics[ ]*= {.*},\)/\L\1/e/teh/e


