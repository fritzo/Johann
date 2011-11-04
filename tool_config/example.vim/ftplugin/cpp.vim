set cindent
set smartindent
set autoindent
set tabstop=2
set shiftwidth=2
set smarttab
set expandtab
set softtabstop=2

"folding
set foldmethod=syntax
set foldnestmax=2
set foldcolumn=3
set columns=83
syntax region cpp_fold start="^{\zs[^}]*\n" end="^\ze}" transparent fold
"syntax region comment_fold start="^\/\*" end="^\*\/" transparent fold
syntax region comment_fold matchgroup=cComment start="^\/\*[^\(\*\/\)]*\n" end="\*\/" transparent fold
syntax match Visibility "\(\<pub\>\|\<pri\>\|\<pro\>\)"
highlight link Visibility Type
highlight cComment cterm=bold ctermfg=4 guibg=bg guifg=#80a0ff
highlight Folded guibg=bg guifg=#333333
highlight FoldColumn guibg=#333333 guifg=#777777

set fillchars="vert:|.fold:\ "
set foldtext='\ '.getline(v:foldstart).'...'.getline(v:foldend)

let Tlist_Exit_OnlyWindow = 1

"shortcuts
map <silent> ,c :s/^/\/\///teh
map <silent> ,u :s/^\/\////teh
map <silent> ,1 :q:set columns=83
map <silent> ,2 :set columns=167:vs
map <silent> ,w :set columns=83
map <silent> ,W :set columns=167

