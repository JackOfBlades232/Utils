if exists(":Files")
    if executable('rg')
        let $FZF_DEFAULT_COMMAND = 'rg --files --hidden --glob "!.git/*"'
    elseif executable('fd')
        let $FZF_DEFAULT_COMMAND = 'fd --type f --hidden --exclude .git'
    endif

    nnoremap <C-f> :Files<CR>
    nnoremap <C-b> :Buffers<CR>
endif
