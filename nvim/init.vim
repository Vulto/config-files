set nocompatible
let mapleader=" "
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
let &undodir = stdpath('state') . '/undo'
call mkdir(&undodir, 'p')
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
set noshowmode
set noconfirm
set nopaste
set signcolumn=yes
set updatetime=250
set completeopt=menuone,noselect,noinsert

let g:c_syntax_for_h = 1
let g:c_comment_strings = 1
let g:c_space_errors = 1

autocmd TermOpen * startinsert
nnoremap <leader>t :belowright split \| terminal<CR>

augroup CIndent
  autocmd!
  autocmd FileType c,cpp setlocal noexpandtab tabstop=8 shiftwidth=8 softtabstop=8 cindent
augroup END

function! ApplyDwmTheme() abort
  let l:name = 'nord'
  let l:nvim = 'nord'
  let l:mode = ''
  let l:active = expand('~/.config/dwm/active')
  if filereadable(l:active)
    for l:line in readfile(l:active)
      if l:line =~# '^name='
        let l:name = trim(split(l:line, '=', 2)[1])
      elseif l:line =~# '^nvim='
        let l:nvim = trim(split(l:line, '=', 2)[1])
      elseif l:line =~# '^mode='
        let l:mode = trim(split(l:line, '=', 2)[1])
      endif
    endfor
  else
    let l:curfile = expand('~/.config/dwm/current')
    if filereadable(l:curfile)
      let l:lines = readfile(l:curfile)
      if !empty(l:lines)
        let l:name = trim(l:lines[0])
        let l:nvim = l:name
      endif
    endif
    let l:tf = expand('~/.config/dwm/themes/') . l:name
    if filereadable(l:tf)
      for l:line in readfile(l:tf)
        if l:line =~# '^nvim='
          let l:nvim = split(l:line, '=', 2)[1]
        endif
      endfor
    endif
  endif
  if empty(l:nvim)
    let l:nvim = l:name
  endif
  if l:mode ==# 'light' || l:nvim ==# 'PaperColor'
    set background=light
  else
    set background=dark
  endif
  execute 'silent! colorscheme' fnameescape(l:nvim)
endfunction

function! s:StatuslineHighlights() abort
  if &background ==# 'light'
    highlight StatusLine   guifg=#303030 guibg=#d0d0d0 ctermfg=236 ctermbg=252 gui=NONE cterm=NONE
    highlight StatusLineNC guifg=#606060 guibg=#d0d0d0 ctermfg=241 ctermbg=252 gui=NONE cterm=NONE
    highlight StatuslineModeNormal   guifg=#ffffff guibg=#005faf ctermfg=15 ctermbg=25  gui=bold cterm=bold
    highlight StatuslineModeInsert   guifg=#ffffff guibg=#00875f ctermfg=15 ctermbg=29  gui=bold cterm=bold
    highlight StatuslineModeVisual   guifg=#ffffff guibg=#875faf ctermfg=15 ctermbg=97  gui=bold cterm=bold
    highlight StatuslineModeReplace  guifg=#ffffff guibg=#af0000 ctermfg=15 ctermbg=124 gui=bold cterm=bold
    highlight StatuslineModeCommand  guifg=#000000 guibg=#d7af00 ctermfg=0  ctermbg=178 gui=bold cterm=bold
    highlight StatuslineModeTerminal guifg=#000000 guibg=#5fafaf ctermfg=0  ctermbg=73  gui=bold cterm=bold
    highlight StatuslineGit          guifg=#005f00 guibg=#c6c6c6 ctermfg=22 ctermbg=251
    highlight StatuslinePath         guifg=#303030 guibg=#d0d0d0 ctermfg=236 ctermbg=252
    highlight StatuslineLine         guifg=#303030 guibg=#bcbcbc ctermfg=236 ctermbg=250
  else
    highlight StatusLine   guifg=#D8DEE9 guibg=#3B4252 ctermfg=7 ctermbg=8 gui=NONE cterm=NONE
    highlight StatusLineNC guifg=#4C566A guibg=#3B4252 ctermfg=8 ctermbg=8 gui=NONE cterm=NONE
    highlight StatuslineModeNormal   guifg=#2E3440 guibg=#88C0D0 ctermfg=0 ctermbg=6  gui=bold cterm=bold
    highlight StatuslineModeInsert   guifg=#2E3440 guibg=#A3BE8C ctermfg=0 ctermbg=2  gui=bold cterm=bold
    highlight StatuslineModeVisual   guifg=#2E3440 guibg=#B48EAD ctermfg=0 ctermbg=5  gui=bold cterm=bold
    highlight StatuslineModeReplace  guifg=#ECEFF4 guibg=#BF616A ctermfg=15 ctermbg=1 gui=bold cterm=bold
    highlight StatuslineModeCommand  guifg=#2E3440 guibg=#EBCB8B ctermfg=0 ctermbg=3  gui=bold cterm=bold
    highlight StatuslineModeTerminal guifg=#2E3440 guibg=#8FBCBB ctermfg=0 ctermbg=14 gui=bold cterm=bold
    highlight StatuslineGit          guifg=#A3BE8C guibg=#434C5E ctermfg=2 ctermbg=8
    highlight StatuslinePath         guifg=#D8DEE9 guibg=#3B4252 ctermfg=7 ctermbg=8
    highlight StatuslineLine         guifg=#D8DEE9 guibg=#434C5E ctermfg=7 ctermbg=8
  endif
endfunction

let s:mode_map = {
      \ 'n':  ['NORMAL',   'StatuslineModeNormal'],
      \ 'no': ['NORMAL',   'StatuslineModeNormal'],
      \ 'i':  ['INSERT',   'StatuslineModeInsert'],
      \ 'ic': ['INSERT',   'StatuslineModeInsert'],
      \ 'ix': ['INSERT',   'StatuslineModeInsert'],
      \ 'v':  ['VISUAL',   'StatuslineModeVisual'],
      \ 'V':  ['V-LINE',   'StatuslineModeVisual'],
      \ "\<C-v>": ['V-BLOCK', 'StatuslineModeVisual'],
      \ 's':  ['SELECT',   'StatuslineModeVisual'],
      \ 'S':  ['S-LINE',   'StatuslineModeVisual'],
      \ "\<C-s>": ['S-BLOCK', 'StatuslineModeVisual'],
      \ 'R':  ['REPLACE',  'StatuslineModeReplace'],
      \ 'Rv': ['V-REPLACE', 'StatuslineModeReplace'],
      \ 'c':  ['COMMAND',  'StatuslineModeCommand'],
      \ 'cv': ['EX',       'StatuslineModeCommand'],
      \ 'ce': ['EX',       'StatuslineModeCommand'],
      \ 't':  ['TERMINAL', 'StatuslineModeTerminal'],
      \ }

function! s:GitDir() abort
  let l:path = expand('%:p:h')
  if empty(l:path) || !isdirectory(l:path)
    return getcwd()
  endif
  return l:path
endfunction

function! GitRefresh() abort
  let l:dir = s:GitDir()
  let l:branch = trim(system('git -C ' . shellescape(l:dir) . ' rev-parse --abbrev-ref HEAD 2>/dev/null'))
  if v:shell_error || empty(l:branch)
    let b:git_status = ''
    return
  endif
  let l:dirty = trim(system('git -C ' . shellescape(l:dir) . ' status --porcelain --ignore-submodules 2>/dev/null'))
  let b:git_status = empty(l:dirty) ? l:branch : l:branch . '*'
endfunction

function! s:StatuslineEscape(text) abort
  return substitute(a:text, '%', '%%', 'g')
endfunction

function! Statusline() abort
  let l:info = get(s:mode_map, mode(), ['NORMAL', 'StatuslineModeNormal'])
  let l:s = '%#' . l:info[1] . '# ' . l:info[0] . ' '
  let l:git = get(b:, 'git_status', '')
  if !empty(l:git)
    let l:s .= '%#StatuslineGit# ' . l:git . ' '
  endif
  let l:name = expand('%:t')
  let l:full = expand('%:p')
  if empty(l:full)
    let l:s .= '%#StatuslinePath# [No Name] %m%r'
  else
    let l:s .= '%#StatuslinePath# ' . s:StatuslineEscape(l:name)
    let l:s .= '  ' . s:StatuslineEscape(l:full) . ' %m%r'
  endif
  let l:s .= '%='
  let l:s .= '%#StatuslineLine# %l/%L '
  return l:s
endfunction

call ApplyDwmTheme()
call s:StatuslineHighlights()
set statusline=%!Statusline()

augroup Statusline
  autocmd!
  autocmd ColorScheme * call s:StatuslineHighlights()
  autocmd BufEnter,BufWritePost,DirChanged,FileChangedShellPost * call GitRefresh()
augroup END

lua << EOF
local uv = vim.uv or vim.loop
local dir = vim.fn.expand('~/.config/dwm')
local w = uv.new_fs_event()
if w then
  w:start(dir, {}, vim.schedule_wrap(function()
    vim.cmd('call ApplyDwmTheme()')
  end))
end

vim.api.nvim_create_autocmd('FileType', {
  pattern = 'c',
  callback = function(ev)
    pcall(vim.treesitter.start, ev.buf, 'c')
  end,
})

vim.api.nvim_create_autocmd('LspAttach', {
  group = vim.api.nvim_create_augroup('c.lsp', { clear = true }),
  callback = function(args)
    local client = vim.lsp.get_client_by_id(args.data.client_id)
    if not client then
      return
    end
    if client:supports_method('textDocument/completion') then
      vim.lsp.completion.enable(true, client.id, args.buf, { autotrigger = true })
    end
    local opts = { buffer = args.buf, silent = true }
    vim.keymap.set('n', 'gd', vim.lsp.buf.definition, vim.tbl_extend('force', opts, { desc = 'LSP definition' }))
    vim.keymap.set('n', '<leader>e', vim.diagnostic.open_float, vim.tbl_extend('force', opts, { desc = 'Line diagnostics' }))
    vim.keymap.set('n', '<leader>q', vim.diagnostic.setqflist, vim.tbl_extend('force', opts, { desc = 'Diagnostics quickfix' }))
  end,
})

vim.lsp.enable('clangd')
EOF

"Leader is <Space>.
"set paste is off so indent, mappings, and completion keep working.

"Syntax
"• Built-in C syntax plus treesitter (parser ships with Neovim 0.11)
"• .h files are treated as C, not C++
"• Extra C highlights: strings in comments, space errors

"LSP (clangd)
"• Hover: K
"• Definition: gd or CTRL-]
"• Rename / actions / references: grn / gra / grr
"• Diagnostics: [d ]d, <Space>e float, <Space>q quickfix
"• Completion auto-triggers in insert; confirm with CTRL-y

"C indent: tabs, tabstop=8, cindent. No format-on-save.

"For Makefile projects without compile_commands.json, drop a compile_flags.txt
"next to the Makefile so clangd can see include paths.
