" Essentials
set nocompatible
set noswapfile
set nobackup

filetype on
filetype plugin on
filetype indent on

" Syntax highlighting
syntax on
set t_Co=256
set termguicolors
let g:gruvbox_bold=1
let g:gruvbox_italic=0
set background=dark
colorscheme gruvbox
hi Normal guibg=NONE ctermbg=NONE

" Custom highlight for keywords and types
autocmd Syntax * syn keyword myTodo NOTE IDEA HUH HACK SPEED OPTIMIZE FEATURE FEAT BUG FIX TEST containedin=.*Comment,vimCommentTitle,cComment,cCommentL,cBlock contained
hi def link myTodo Todo
autocmd Syntax * syn keyword myType u8 s8 u16 s16 u32 s32 u64 s64 f32 f64
hi def link myType Type

" Line numbers
set nu
set relativenumber
set scrolloff=8

" Search
set incsearch
set nohlsearch

" Wild menu & file find
set path+=**
set wildmenu

" Tabs config
set shiftwidth=4
set softtabstop=4
set expandtab

set shiftwidth=4
set tabstop=4

" Tags
command! MakeTags !ctags -R .

" File browser
let g:netrw_banner=0
" TODO: incomplete

" Undo
if !isdirectory($HOME."/.vim")
    call mkdir($HOME."/.vim", "", 0770)
endif
if !isdirectory($HOME."/.vim/undo-dir")
    call mkdir($HOME."/.vim/undo-dir", "", 0700)
endif
set undodir=~/.vim/undo-dir
set undofile

" Other
set colorcolumn=80
set cursorline
set mouse=a
set history=1000
set nowrap

" Lang specific
au BufNewFile,BufRead *.pp set filetype=pascal
au BufNewFile,BufRead *.inc set filetype=nasm
au BufNewFile,BufRead *.asm set filetype=nasm

" Cursor
let &t_SI = "\<Esc>[5 q"
let &t_SR = "\<Esc>[4 q"
let &t_EI = "\<Esc>[2 q"

" Clipboard  
set clipboard+=unnamedplus

" Remaps
let mapleader = " "
nnoremap <leader>pv :Ex<CR>
nnoremap <C-f> :find 
vnoremap J :m '>+1<CR>gv=gv
vnoremap K :m '<-2<CR>gv=gv
nnoremap <Leader>y "+y
nnoremap <Leader>p "+p
nnoremap <Leader>Y "*y
nnoremap <Leader>P "*p
