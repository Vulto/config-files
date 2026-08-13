set nocompatible
filetype plugin on
filetype indent on
set showmatch
set encoding=utf-8
syntax on
set noerrorbells
set smartindent
set smartcase
set noswapfile
set nobackup
set undodir=/home/vulto/.config/nvim/undodir
set undofile
set incsearch

set hlsearch   " highlight all results
nnoremap <Esc> :nohlsearch<CR> " clear highlight with escape

set scrolloff=5
set cursorline
set number relativenumber
set splitbelow
set cmdheight=0
set laststatus=3
set noconfirm
set paste

autocmd termOpen * startinsert
let mapleader=" "
nnoremap <C-t> :belowright split \| terminal<CR>

highlight Normal guibg=NONE ctermbg=NONE
highlight NormalNC guibg=NONE ctermbg=NONE
