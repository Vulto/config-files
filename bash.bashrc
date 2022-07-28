# /etc/bash.bashrc
#
# If not running interactively, don't do anything
[[ $- != *i* ]] && return

[[ $DISPLAY ]] && shopt -s checkwinsize

[[ $UID = 1000 ]] && PS1="\n  \w  \n   → " || PS1="\n \e[1;31m # [ \w ] \e[m \n    "

alias ls="ls --color=auto"
alias ll="ls -lhtr"
alias l="ls -1"
alias cat="bat"
alias vi="vim"
alias v="vim"
alias bc="bc -q"
alias q="exit"

# \C-? < control+some character
# \e? < Alt+some character
# \ew? < Alt+Shift+some character

bind "\C-x":kill-line
bind "\C-v":yank
bind -x '"\ef":"oi f"'
bind -x '"\ep":"oi pdf"'
bind -x '"\eh":"oi h"'
bind -x '"\ec":"oi c"'
bind -x '"\ed":"pushd `fd -td . | nmenu`"'
bind -x '"\es":"oi oprs"'
bind -x '"\et":"oi todo"'
bind -x '"\ev":"oi v"'
bind -x '"\C-r":"oi hist"'
