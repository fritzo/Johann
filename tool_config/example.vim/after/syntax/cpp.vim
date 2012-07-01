
"attempts to speed up syntax highlighting
set synmaxcol=160
syntax sync minlines=10
let c_no_if0 = 1

set formatoptions=

syntax case match

syntax keyword Todo TODO
syntax keyword Todo TODO
syntax keyword Todo XXX
syntax keyword Todo HACK
syntax keyword Todo FIXME
syntax keyword Todo DEBUG
syntax keyword Todo WORKING
syntax keyword Todo DEPRECATED
syntax keyword Todo OLD

syntax match Debug "\<Assert[0-9AWVCP]\?\>"
syntax match Debug "\<Error\>"
syntax match Debug "logger[.][a-z0-9]*()"

syntax match Debug "\<LOG[0-9]*\>"
syntax match Debug "\<DEBUG[0-9]*\>"
syntax match Debug "\<ERROR[0-9]*\>"
syntax match Debug "\<WARN[0-9]*\>"
syntax match Debug "\<ASSERT[A-Z0-9_]*\>"
syntax match Debug "\<EXPECT[A-Z0-9_]*\>"
syntax match Debug "\<TEST[A-Z0-9_]*\>"
syntax match Debug "\<PRINT[A-Z0-9_]*\>"

syn keyword cConstant INFINITY
syn keyword cConstant NAN

syn keyword cLabel result
syn keyword cLabel Result

highlight Separator gui=bold guifg=#80a0ff
syntax match Separator "[^-]\zs---\+" contained containedin=cComment,cCommentL
syntax match Separator "[^=]\zs===\+" contained containedin=cComment,cCommentL

highlight ScmConflict gui=bold guibg=red guifg=bg
syntax match ScmConflict "^<<<<.*"
syntax match ScmConflict "^====.*"
syntax match ScmConflict "^>>>>.*"

highlight TypeDef guifg=seagreen

"----( Johann )----

syntax keyword TypeDef Short
syntax keyword TypeDef Int
syntax keyword TypeDef Long
syntax keyword TypeDef Float
syntax keyword TypeDef Double

"----( Kazoo )----

syntax keyword Type string
syntax keyword Type Id
syntax keyword Type Real
syntax keyword Type Complex
syntax keyword Type Vector
syntax keyword Type Reals
syntax keyword Type Complexes
syntax keyword Type Real4
syntax keyword Type Real8
syntax keyword Type Real12
syntax keyword Type Real16

"----( Toyon )----

syntax match TypeDef "\<T[TSCDR][A-Za-z]\+\>"

highlight ToyonStyleError guibg=red
syntax match ToyonStyleError "\s\+$"
syntax match ToyonStyleError "\n\n\n\+$"
"syntax match ToyonStyleError "^#endif$"
"syntax match ToyonStyleError "^#else$"
syntax match ToyonStyleError "\<TRC_[A-Z0-9]\+_[A-Z0-9]\+_H\>"
syntax match ToyonStyleError "\<m\?Num[A-Z][A-Za-z]*\>"
"syntax match TotonStyleError "$public:"
"syntax match TotonStyleError "$private:"
"syntax match TotonStyleError "$protected:"

