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
autocmd Syntax * syn keyword myType u8 s8 u16 s16 u32 s32 u64 s64 f32 f64 u32f s32f i8 i16 i32 i64 i64f b32 vec2 vec3 vec4 uvec2 uvec3 uvec4 ivec2 ivec3 ivec4 svec2 svec3 svec4 vec2f vec3f vec4f vec2d vec3d vec4d vec2i vec3i vec4i vec2u vec3u vec4u vec2s vec3s vec4s
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
au BufNewFile,BufRead *.frag.inc,*.vert.inc,*.comp.inc,*.glsl.inc set ft=glsl

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

if g:os == "Windows"
    nnoremap <Leader>b :!build\build.bat<CR>
else
    nnoremap <Leader>b :make prog<CR>
endif

if has('nvim')
    call plug#begin()

    Plug 'neoclide/coc.nvim', {'branch': 'release'}
    Plug 'nvim-lua/plenary.nvim'
    Plug 'nvim-telescope/telescope.nvim', { 'branch': '0.1.x' }
    Plug 'nvim-telescope/telescope-fzf-native.nvim', { 'do': 'cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release && cmake --install build --prefix build' }
    Plug 'tikhomirov/vim-glsl'

    call plug#end()

    " @TODO(PKiyashko): go over coc & telescope remap

    " COC

    " Use `[g` and `]g` to navigate diagnostics
    " Use `:CocDiagnostics` to get all diagnostics of current buffer in location list
    nmap <silent> [g <Plug>(coc-diagnostic-prev)
    nmap <silent> ]g <Plug>(coc-diagnostic-next)

    " GoTo code navigation
    nmap <silent> gd <Plug>(coc-definition)
    nmap <silent> gy <Plug>(coc-type-definition)
    nmap <silent> gi <Plug>(coc-implementation)
    nmap <silent> gr <Plug>(coc-references)

    nmap <silent> <C-]> <Plug>(coc-definition)

    " Symbol renaming
    nmap <leader>rn <Plug>(coc-rename)

    " Formatting selected code
    xmap <leader>f <Plug>(coc-format-selected)
    nmap <leader>f <Plug>(coc-format-selected)

    " Add `:Format` command to format current buffer
    command! -nargs=0 Format :call CocActionAsync('format')

    " Using Lua functions
    nnoremap <leader>ff <cmd>lua require('telescope.builtin').find_files()<cr>
    nnoremap <leader>fg <cmd>lua require('telescope.builtin').live_grep()<cr>
    nnoremap <leader>fb <cmd>lua require('telescope.builtin').buffers()<cr>
    nnoremap <leader>fh <cmd>lua require('telescope.builtin').help_tags()<cr>

    nnoremap <C-f> <cmd>lua require('telescope.builtin').git_files()<cr>

endif
