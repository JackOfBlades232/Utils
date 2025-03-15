" Essentials
set nocompatible
set noswapfile
set nobackup
set nowritebackup
set exrc

filetype on
filetype plugin on
filetype indent on

if !exists("g:os")
    if has("win64") || has("win32") || has("win16")
        let g:os = "Windows"
    else
        let g:os = substitute(system('uname'), '\n', '', '')
    endif
endif

if has("gui_running")
    set backspace=indent,eol,start
    set guifont=Cascadia\ Mono:h14
    set guioptions=Ace
endif

set guicursor=n-v-c-sm:block,i-ci-ve:ver15,r-cr-o:hor20

" Syntax highlighting
syntax on
set t_Co=256
set termguicolors
let g:gruvbox_bold=1
let g:gruvbox_italic=0
set background=dark
colorscheme gruvbox

" Custom highlight for keywords and types
autocmd Syntax * syn keyword myTodo NOTE IDEA HUH HACK SPEED OPTIMIZE FEATURE FEAT BUG FIX TEST WIP containedin=.*Comment,vimCommentTitle,cComment,cCommentL,cBlock contained
hi def link myTodo Todo
autocmd Syntax * syn keyword myType uint u8 s8 u16 s16 u32 s32 u64 s64 f32 f64 u32f s32f i8 i16 i32 i64 i64f b32
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

set pumheight=15
set pumwidth=60

" Tags
command! MakeTags !ctags -R .

" File browser
let g:netrw_banner=0
" @TODO(PKiyashko): incomplete

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
au BufNewFile,BufRead *.asm set filetype=nasm
au BufNewFile,BufRead *.frag.inc,*.vert.inc,*.comp.inc,*.glsl.inc set ft=c
au BufNewFile,BufRead *.dshl set ft=c

" Cursor
let &t_SI = "\<Esc>[5 q"
let &t_SR = "\<Esc>[4 q"
let &t_EI = "\<Esc>[2 q"

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
nnoremap <Leader>t :MakeTags<CR>

nnoremap <Leader>1 1gt
nnoremap <Leader>2 2gt
nnoremap <Leader>3 3gt
nnoremap <Leader>4 4gt
nnoremap <Leader>5 5gt
nnoremap <Leader>6 6gt
nnoremap <Leader>7 7gt
nnoremap <Leader>8 8gt
nnoremap <Leader>9 9gt

if g:os == "Windows"
    nnoremap <Leader>b :!build\build.bat<CR>
else
    nnoremap <Leader>b :make prog<CR>
endif

if exists("g:neovide")
    let g:neovide_cursor_animation_length = 0
    let g:neovide_position_animation_length = 0
    let g:neovide_scroll_animation_length = 0
    let g:neovide_cursor_trail_size = 0
    set guifont=Cascadia\ Mono:h14:#e-subpixelantialias
endif
