syntax case match

syntax keyword Type True
syntax keyword Type False
syntax keyword Type None

syntax keyword Todo TODO
syntax keyword Todo TODO
syntax keyword Todo XXX
syntax keyword Todo HACK
syntax keyword Todo FIXME
syntax keyword Todo DEBUG
syntax keyword Todo WORKING
syntax keyword Todo DEPRECATED
syntax keyword Todo OLD

highlight Separator gui=bold guifg=#80a0ff
syntax match Separator "---\+" contained containedin=pythonComment
syntax match Separator "===\+" contained containedin=pythonComment

highlight ScmConflict gui=bold guibg=red guifg=bg
syntax match ScmConflict "^<<<<.*"
syntax match ScmConflict "^====.*"
syntax match ScmConflict "^>>>>.*"

highlight StyleError guibg=red
syntax match StyleError "\s\+$"
syntax match StyleError "\n\n\n\+$"

