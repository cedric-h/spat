source ~/.vimrc

set path=$HOME/spat/src/**
set path+=$HOME/spat/vendored/SDL/**

set ts=4
set sw=4
set expandtab

noremap <nowait> \| :vimgrep  src/**/*<C-Left><C-Left>

augroup highlight_sdl
  autocmd!
  autocmd FileType cpp syntax match HighlightSDL /\<SDL_Rect\>\|\<SDL_Point\>\|\<SDL_Window\>\|\<SDL_Surface\>\|\<SDL_Event\>/
  autocmd FileType cpp highlight link HighlightSDL cType

  autocmd FileType cpp syntax match HighlightGadget /\<gadget_ID\>\|\<gadget_Do\>\|\<gadget_Gadget\>\|\<gadget_GadgetKind\>\|\<gadget_Action\>/
  autocmd FileType cpp highlight link HighlightGadget cType
augroup END
