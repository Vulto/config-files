return {
  cmd = {
    'clangd',
    '--background-index',
    '--clang-tidy',
    '--header-insertion=never',
  },
  filetypes = { 'c', 'cpp' },
  root_markers = {
    'compile_commands.json',
    'compile_flags.txt',
    '.clangd',
    'Makefile',
    '.git',
  },
}
