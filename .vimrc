set nocompatible
filetype plugin on
filetype indent on
set mouse=
set showmatch
set encoding=utf-8
syntax on
set noerrorbells
set signcolumn=no
set tabstop=2 softtabstop=2
set shiftwidth=2
set smartindent
set smartcase
set noswapfile
set nobackup
set undodir=/home/vulto/.vim/undodir
set undofile
set incsearch
set hlsearch
set scrolloff=5
set cursorline
set colorcolumn=70
set number relativenumber
set nohlsearch
set splitbelow
set cmdheight=1
set noshowcmd  "Get rid of display of last command"
set shortmess+=F  "Get rid of the file name"
set laststatus=0
set noconfirm
colorscheme gruvbox
set background=dark
set paste
set noshowmode

hi Normal ctermbg=NONE
hi Cursor ctermbg=NONE ctermfg=NONE cterm=NONE

hi CursorLine ctermbg=NONE ctermfg=NONE cterm=NONE
hi LineNr ctermbg=NONE ctermfg=gray cterm=NONE
hi CursorLineNR ctermbg=NONE ctermfg=NONE cterm=bold

hi ColorColumn ctermbg=NONE cterm=NONE
hi SignColumn ctermbg=NONE cterm=Bold

hi TabLineFill ctermbg=NONE ctermfg=white
hi TabLine ctermbg=NONE ctermfg=gray cterm=italic
hi TabLineSel ctermbg=NONE ctermfg=NONE cterm=bold,italic
hi Title ctermbg=NONE ctermfg=NONE
hi BufTabLineInactive ctermbg=NONE ctermfg=gray
hi BufTabLineActive ctermbg=NONE ctermfg=white

set foldcolumn=0
hi foldcolumn ctermbg=NONE
hi VertSplit ctermbg=NONE ctermfg=grey

set splitright
let mapleader= " "

let g:netrw_banner = 0
let g:netrw_liststyle = 3
let g:netrw_browse_split = 3
let g:netrw_altv = 1
let g:netrw_winsize = 15

"keybind to terminal in insert mode
autocmd TermOpen * startinsert
nnoremap <silent> <Leader>t :sp term://bash <CR>

nnoremap <leader>h :wincmd h<CR>
nnoremap <leader>j :wincmd j<CR>
nnoremap <leader>k :wincmd k<CR>
nnoremap <leader>l :wincmd l<CR>
nnoremap <silent> <Leader>+ :vertical resize +15<CR>
nnoremap <silent> <Leader>- :vertical resize -15<CR>

"mksession name.vim
"Vexplore
