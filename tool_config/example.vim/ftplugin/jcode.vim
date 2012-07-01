set autoindent
set tabstop=2
set shiftwidth=2
set shiftround
set smarttab
set expandtab
set softtabstop=2
set nojoinspaces
"set textwidth=80

set foldmethod=indent
set foldnestmax=1
set foldcolumn=2
set columns=82
set foldlevel=2

"environment shortcuts
map ,c :s/^/####//teh
map ,u :s/^###.///teh

"set iskeyword="a-z,A-Z,48-57,_,'"

"syntax highlighting
syntax sync fromstart
highlight Folded guibg=bg guifg=#333333
highlight FoldColumn guibg=#333333 guifg=#777777
set fillchars="vert:|.fold:\ "
set foldtext=getline(v:foldstart).'...'

"edit markings
highlight MARKER guibg=orange guifg=black gui=none
highlight cComment guifg=darkblue gui=none
highlight ShortComment guifg=darkblue gui=none
highlight AntiComment guifg=black gui=none
highlight StrongComment guifg=gray gui=none
highlight Markup guifg=darkmagenta gui=none
highlight Ascii guifg=brown gui=none
syntax match MARKER "\(XXX\|FIXME\|WORKING\|TODO\|OLD\|???\).*"
syntax region cComment start="^[ ]*###[{\])]" end="^[ ]*###[}([]" contains=Markup,MARKER,AntiComment,StrongComment,Ascii
syntax region ShortComment start="\(^\|[^#]\)\zs#\ze[^#]" end="[#\n]" contains=Markup,MARKER containedin=ALLBUT,cComment,AntiComment,StrongComment,Ignored keepend
syntax region AntiComment start="\(^\|[^#]\)\zs#\ze[^#]" end="[#\n]" contains=ALLBUT,ShortComment,texCommand containedin=cComment contained keepend
syntax region StrongComment start="^[ ]*####" end="\n" contains=NONE
syntax cluster Comment add=cComment,ShortComment
syntax region Markup start="^[ ]*###[duUkmglir]" end="\n" contains=NONE
syntax region Ascii start="^[ ]*###a" end="^[ ]*###a"

"commands
highlight Command guifg=seagreen gui=none
highlight Echo guifg=blue gui=bold
syntax match Command "!\([0-9]\+ \)\?[a-z]\+"
syntax match Echo "!log .*"

"weighted sets
highlight WeightError guifg=white guibg=red gui=none
highlight Weight guifg=seagreen gui=none
highlight Float guifg=magenta gui=none
highlight Boolean guifg=magenta gui=none
syntax match WeightError "@"
syntax match Weight "@\ze[ ]*[0-9]\+\([.][0-9]\+\)\?"
syntax match Float "[^a-zA-Z_]\zs[0-9]*[.][0-9]\+"
syntax keyword true false fail error success containedin=Keyword contained

"term syntax
highlight Keyword guifg=black gui=none
highlight Unknown guibg=orange guifg=black gui=bold
highlight Error guibg=orange guifg=black gui=bold
highlight Atom gui=bold
highlight Type guifg=seagreen gui=bold
syntax match Keyword "[A-Za-z_']\+[A-Za-z_0-9']*" contains=Error,Atom,Type
syntax match Unknown "???"
syntax keyword Error XXX containedin=Keyword contained
syntax keyword Atom let A B C E F J K I R S T W Y U V O P Q D Phi Psi Delta Omega Join rand if assert sel inl inr atom app atoms extra_atoms test_code if_Bot if_Top if_less containedin=Keyword contained
syntax keyword Type nil any div Simple unit semi bool num nat sset prod sum maybe code containedin=Keyword contained

"special constructions
highlight Selector guifg=magenta
highlight Blank gui=bold
highlight Number guifg=magenta gui=none
highlight Dot gui=none
highlight Quoted guifg=darkred
syntax match Selector "\<\[[0-9]\+\/[0-9]\+\]\>"
syntax match Blank "\<_\>"
syntax match Number "[0-9]\+"
syntax match Dot "[.]"
syntax match Quoted "\"[^"]\+\""

"end-of-file
highlight Ignored guifg=gray guibg=white
syntax region Ignored matchgroup=Ignored start="^exit" end="\%$" contains=NONE

"text stuff
highlight Emph guifg=red gui=underline
syntax match Emph "\([ ()-/\n{}]\)\@<=_[a-zA-Z0-9(][-a-zA-Z0-9'(),_]\+_\([ ,.?!;:()-/\n'{}]\)\@=" contained containedin=@Comment
highlight Bullet guifg=darkred gui=bold
syntax match Bullet "  \* " contained containedin=@Comment

"latex stuff
syntax match texCommand "\\[a-zA-Z]\+" contained containedin=@Comment
highlight texCommand guifg=darkred gui=bold
syntax match texComment1 "^%.*" contained containedin=@Comment
highlight texComment1 guifg=blue gui=italic
syntax match texComment2 "[^\\]\zs%.*" contained containedin=@Comment
highlight texComment2 guifg=blue gui=italic
syntax match texProblem "^\\Prob[ ]*[^ ]*[ ]*" contained containedin=@Comment
highlight texProblem guifg=white guibg=black
syntax match texTakeout "^\\takeout\(.*\n\)\{-}\_^}" contained containedin=@Comment
highlight texTakeout guifg=blue gui=italic
