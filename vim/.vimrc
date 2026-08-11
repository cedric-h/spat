source ~/.vimrc

set path=$HOME/spat/src/**
set path+=$HOME/spat/vendored/SDL/**

set ts=4
set sw=4
set expandtab

noremap <nowait> \| :vimgrep  src/**/*<C-Left><C-Left>
