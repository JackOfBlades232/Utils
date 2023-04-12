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
let g:gruvbox_contrast_dark='hard'
let g:gruvbox_transparent_bg='1'
let g:gruvbox_italic=0
set background=dark
colorscheme gruvbox
hi Normal guibg=NONE ctermbg=NONE

" Line numbers
set nu
set relativenumber
set scrolloff=8

" Search
set incsearch

" Tabs config
set shiftwidth=4
set softtabstop=4
set expandtab

set shiftwidth=4
set tabstop=4

set colorcolumn=80

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
set cursorline
set mouse=a
set history=1000

" Lang specific
au BufNewFile,BufRead *.pp set filetype=pascal
au BufNewFile,BufRead *.inc set filetype=nasm
au BufNewFile,BufRead *.asm set filetype=nasm

" Cursor
let &t_SI = "\<Esc>[5 q"
let &t_SR = "\<Esc>[4 q"
let &t_EI = "\<Esc>[2 q"

" Remaps
let mapleader = " "
nnoremap <leader>pv :Ex<CR>
vnoremap J :m '>+1<CR>gv=gv
vnoremap K :m '<-2<CR>gv=gv
