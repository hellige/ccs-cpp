" Vim syntax file
" Language: CCS
" Maintainer: Matt Hellige <matt@immute.net>, Matt Godbolt <matt@godbolt.org>
" Last change: 2015 February 9th

if exists("b:current_syntax")
    finish
endif

syn clear
syn case match

syn match ccsError '@\w*'
syn match ccsKeywords '@import\>' skipwhite nextgroup=ccsString
syn match ccsKeywords '@constrain\>' skipwhite
syn match ccsKeywords '@context\>' skipwhite nextgroup=ccsParens
syn match ccsModifiers '@override\>' skipwhite
" needs to be high up as it is spectacularly oversensitive
syn match ccsConstraint '\<\w\+\>'

syn region ccsBlock start="{" end="}" fold transparent
syn match ccsComment "//.*$"
syn region ccsComment start="/\*" end="\*/" contains=ccsComment
syn region ccsParens start="(" end=")" transparent

syn region ccsString start='"' end='"' contained contains=ccsInterpolant,ccsEscape
syn region ccsString start='\'' end='\'' contained contains=ccsInterpolant,ccsEscape
syn region ccsInterpolant start='${' end='}' contained
syn match ccsEscape '\\[tnr'"\$]' contained

syn region ccsConstraintString start='"' end='"'
syn region ccsConstraintString start='\'' end='\''

" Integer with - + or nothing in front
syn match ccsNumber '\d\+' contained
syn match ccsNumber '[-+]\d\+' contained

" Floating point number with decimal no E or e (+,-)
syn match ccsNumber '\d\+\.\d*' contained
syn match ccsNumber '[-+]\d\+\.\d*' contained

" Floating point like number with E and no decimal point (+,-)
syn match ccsNumber '[-+]\=\d[[:digit:]]*[eE][\-+]\=\d\+' contained
syn match ccsNumber '\d[[:digit:]]*[eE][\-+]\=\d\+' contained

" Floating point like number with E and decimal point (+,-)
syn match ccsNumber '[-+]\=\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+' contained
syn match ccsNumber '\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+' contained

syn keyword ccsBoolean true false skipwhite contained

syn match ccsIdentifier '\<\w\+\>\(\s*=\)\@='
syn match ccsDefId '\<\w\+\>' contained

syn match ccsOperator '[.,>:|{}*()]'

syn match ccsOperator '=' skipwhite nextgroup=ccsNumber,ccsBoolean,ccsString,ccsDefId

let b:current_syntax = "ccs"
hi def link ccsError Error
hi def link ccsKeywords Statement
hi def link ccsModifiers Type
hi def link ccsConstraint Type
hi def link ccsConstraintString Type
hi def link ccsComment Comment
hi def link ccsString Constant
hi def link ccsNumber Constant
hi def link ccsBoolean Constant
hi def link ccsDefId Constant
hi def link ccsIdentifier Identifier
hi def link ccsOperator Operator
hi def link ccsInterpolant PreProc
