#
# /etc/bash.bashrc
#
# If not running interactively, don't do anything
[[ $- != *i* ]] && return

[[ $DISPLAY ]] && shopt -s checkwinsize

PS1="\n[ \w ]\n    "

# Setup fzf
if [[ ! "$PATH" == */home/vulto/.config/fzf/bin* ]]; then
  export PATH="${PATH:+${PATH}:}/home/vulto/.fzf/bin"
fi

# Auto-completion
# ---------------
[[ $- == *i* ]] && source "/home/vulto/.config/fzf/shell/completion.bash" 2> /dev/null

# Key bindings
# ------------
source "/home/vulto/.config/fzf/shell/key-bindings.bash"

alias steam="steam -nofriendsui -nochatui"
