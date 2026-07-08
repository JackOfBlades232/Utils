local ok, fff = pcall(require, "fff")

if ok then
    fff.setup({})
    vim.keymap.set("n", "<C-f>", fff.find_files, {
        desc = "Find files",
    })
    vim.keymap.set("n", "<C-S-f>", fff.live_grep, {
        desc = "Live grep",
    })
end
