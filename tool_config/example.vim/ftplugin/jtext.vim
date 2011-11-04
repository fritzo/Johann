set autoindent
set tabstop=2
set shiftwidth=2
set smarttab
set expandtab
set softtabstop=2
set autoindent
set textwidth=80
set matchpairs=(:),[:],<:>,{:}

"shortcuts
map <silent> ,c :s/^/####//teh
map <silent> ,u :s/^###.///teh
map <silent> ,p :%s/^####.*\n//:ha > print.psu/teh

"syntax params
syntax sync minlines=100

"edit markings
hi XXX guibg=yellow guifg=black
syntax keyword XXX XXX
syntax keyword XXX FIXME
syntax keyword XXX WORKING
syntax keyword XXX OLD
syntax keyword XXX TODO
syntax keyword XXX LATER
syntax keyword XXX EDIT
syntax match XXX "???"

"text formatting
hi Quote guifg=#bb6666
hi Emph guifg=#ff4444 gui=underline
hi Mathbf guifg=fg gui=bold
hi Comment cterm=bold ctermfg=4 gui=italic guifg=#80a0ff
hi InlineMath guifg=#44aa44 gui=none
hi Takeout1 guifg=#333333
hi JtextDef guifg=gray
hi JcodeDef guifg=orange gui=none
hi MathBlock guifg=#333333
hi LatexDef guifg=gray
hi Dollar guifg=bg gui=none
hi Http guifg=#8888ff
syntax match Quote "\"[^\"]*\""
syntax match Emph "\([ ()-/\n{}]\)\@<=_[a-zA-Z0-9(][-a-zA-Z0-9'(),_]\+_\([ ,.?!;:()-/\n'{}]\)\@="
syntax match Mathbf "\([ ()-/\n{}]\)\@<=_[a-zA-Z]_\([ ,.?!;:()-/\n'{}]\)\@="
syntax match Comment "#[^#]*\n"
syntax match InlineMath "#[^#]\+#"
syntax match Takeout1 "####.*"
syntax match JtextDef "###[^#<>[\]()an].*"
syntax match MathBlock "^ *\\[[\]]"
syntax match LatexDef "\\newcommand.*"
syntax match Http "\<https\?://\S\+"

"code block
hi Jcode       guifg=#44aa44
hi Jmath       guifg=#44aa44
hi Jnumb       guifg=#44aa44
hi Jascii      guifg=#bb6666
hi JCvariable  guifg=#44aa44
hi JCmeta      guifg=#bb6666
"hi JClogic     guifg=#bb6666
hi JCcomment   guifg=#80a0ff gui=italic
hi JCpause     guifg=#333333
hi JCkomment   guifg=#333333
hi JCmarker    guifg=#333333
hi JCedit      guibg=#aa8800 guifg=black gui=none
hi JCwarning   guibg=#aa8800 guifg=black gui=none
hi JNumber     guifg=fg gui=bold
hi JCbar       guibg=lightgrey guifg=bg
"hi JCheck      guifg=gray guibg=bg gui=none
syntax region Jcode start="^[ ]*###<" end="^[ ]*###>" keepend
syntax region Jmath start="^[ ]*###[[(=\-]" end="^[ ]*###[\])=\-]" keepend
syntax region Jnumb start="^[ ]*###n" end="^[ ]*###n" keepend
syntax region Jascii start="^[ ]*###a" end="^[ ]*###a" keepend
syntax cluster J_ add=Jcode,Jmath,Jnumb
syntax match JCvariable "[A-Za-z0-9_']\+" contained containedin=@J_
syntax match JCcomment "#.*" contained containedin=@J_
syntax match JCpause "#PAUSE" contained containedin=@J_
syntax match JCkomment "####.*" contained containedin=@J_
syntax match JCmeta "!\([0-9]\+ \)\?[a-z]\+" contained containedin=Jcode
"syntax match JClogic "\<and\|or\|not\>" contained containedin=@J_
syntax match JCmarker "###[<>[\]()-=an]" contained containedin=@J_,Jascii
syntax match JCedit "#\(XXX\|FIXME\|WORKING\|LATER\|OLD\|FAILED\|UNKNOWN\).*" contained containedin=@J_
syntax match JCwarning "???\|XXX" contained containedin=Jcode
syntax match JCbar "^ \ze[ ]*[^ #]" contained containedin=Jcode
syntax match JNumber "^\s*[0-9.]\+[.]\s" contained containedin=Jnumb
"syntax match JCheck "!check.*" contained containedin=Jcode

"headings
hi Section guifg=white gui=bold
hi SubSection guifg=white gui=bold
hi SubSubSection guifg=white gui=bold
hi Block guibg=bg guifg=#80a0ff gui=bold
hi Takeout2 guifg=#333333
syntax match Section "^\([A-Z][a-z]\+\|A\)\([,:;]*[ \-'/][A-Za-z\"]\+\)*\.[ ]\@="
syntax match SubSection "^  \([A-Z][a-z]\+\|A\)\([,;]*[ \-'/][A-Za-z\"]\+\)*\.[ ]\@="
syntax match SubSubSection "^    \([A-Z][a-z]\+\|A\)\([,;]*[ \-'/][A-Za-z\"]\+\)*\.[ ]\@="
syntax match Block "\(^[ ]*\)\@<=\([A-Z][a-z]\+\)\([ ][A-Z][a-z]\+\)*\([ ][0-9]\+\([.][0-9]\+\)*\)\?:[ ]\@="
syntax match Takeout2 "^\( *\)Takeout: .*\(\n\|\1  .*\)*"

"document structure
hi NumberedBullets guifg=#80a0ff gui=bold
hi Bullet guifg=#80a0ff gui=bold
hi Bullet2 guifg=#80a0ff gui=bold
hi Date guifg=#bb6666 gui=bold
hi EndProof guibg=bg guifg=#80a0ff gui=bold
syntax match NumberedBullets "([A-Z][0-9][A-Z0-9:.+-?']*)"
syntax match Bullet "^[ ]\+[*]"
syntax match Bullet2 "^[ ]*(\([a-z]\|[0-9]\+\|[iv]\+\)\([.]\([a-z]\|[0-9]\+\|[iv]\+\)\)*)[ ]"
syntax match Date "(20[0-1][0-9]\(-[0-9]\)\?:[0-9:,\-a-z]*)"
syntax match EndProof "\[\]\n"

"math notation
hi VBar1 gui=bold
hi VBar2 gui=bold
hi VBar3 gui=bold
hi Comma guifg=#44aa44 gui=bold
syntax match VBar1 "--\+"
syntax match VBar2 "==\+"
syntax match VBar3 "__\+"
syntax match Comma ",\n" containedin=J_,Jascii

"inline johann code
hi UserPrompt guifg=orange gui=bold
hi JohannPrompt guifg=#44aa44 gui=bold
syntax match UserPrompt "^[ ]*(U)"
syntax match JohannPrompt "^[ ]*(J)"

"latex stuff
hi InlineLatex guifg=#44aa44 gui=none
hi BlockLatex1 guifg=#44aa44 gui=none
hi BlockLatex2 guifg=#44aa44 gui=none
hi BlockLatex3 guifg=#44aa44 gui=none
hi LatexMarker guifg=#333333
syntax match InlineLatex "\$[^$]\+\$"
syntax region BlockLatex1 start="\(^\|[^\\]\)\\\[" end="\\\]" keepend
syntax region BlockLatex2 start="\$\$" end="\$\$" keepend
syntax region BlockLatex3 start="\\begin{align\*}" end="\\end{align\*}" keepend
syntax cluster BlockLatex add=BlockLatex1,BlockLatex2,BlockLatex3
syntax match LatexMarker "\$\$" contained containedin=@BlockLatex

"syntax match texCommand "\\[a-zA-Z]\+"
"syntax match texComment "%.*"
"hi texCommand guifg=darkred gui=bold
"hi texComment guifg=#80a0ff gui=italic

hi texTakeout guifg=#333333
hi Pause guifg=#333333
hi Alert guifg=red
syntax match texTakeout "^\\takeout\(.*\n\)\{-}\_^}"
syntax match Pause "\<PAUSE\|SPACE\|BREAK\|NEWPAGE\>"
syntax match Alert "\\alert[^{]*{[^}]*}"

set foldmethod=indent
set foldnestmax=2
set foldcolumn=3
set columns=83
hi Folded guibg=bg guifg=#333333
hi FoldColumn guibg=#333333 guifg=#777777
set fillchars="vert:|.fold:\ "
set foldtext=getline(v:foldstart).'...'
set foldignore=
set foldlevel=2

"environment shortcuts
map ,, A###<  <BS>###><Up><End>a
map ,[ A###[  <BS>###]<Up><End>a
map ,9 A###(  <BS>###)<Up><End>a
map ,- A###-  <BS>###-<Up><End>a
map ,= A###=  <BS>###=<Up><End>a
map ,a A###a  <BS>###a<Up><End>a
map ,n A###n  <BS>###n<Up><End>a

"folding shortcuts
map <silent> ,w :set foldnestmax=2:set foldcolumn=3:set columns=83
map <silent> ,W :set foldnestmax=4:set foldcolumn=5:set columns=85
map <silent> ,1 :q:set columns=83
map <silent> ,2 :set columns=167:vs

"jump to end of file (useful for note-taking)
$

