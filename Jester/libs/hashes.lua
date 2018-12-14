local M = {}

M.INPUT_ACQUIRE_FOCUS = hash("acquire_input_focus")
M.INPUT_RELEASE_FOCUS = hash("release_input_focus")

M.MSG_ENABLE = hash("enable")
M.MSG_DISABLE = hash("disable")

M.MSG_LOAD_PROXY = hash("load_proxy")

M.MSG_PROXY_LOADED = hash("proxy_loaded")
M.MSG_ASYNC_LOAD = hash("async_load")
M.MSG_UNLOAD = hash("unload")

M.MSG_JESTER_INIT = hash("jester_init")
M.MSG_INIT = hash("init")
M.MSG_SHOW = hash("jester_show")
M.MSG_HIDE = hash("jester_hide")
M.MSG_PAUSE = hash("jester_pause")
M.MSG_RESUME = hash("jester_resume")



M.MSG_SHOW = hash("jester_show")
M.MSG_SHOW_IN = hash("jester_show_in")
M.MSG_SHOW_OUT = hash("jester_show_out")
M.MSG_BACK_IN = hash("jester_back_in")
M.MSG_BACK_OUT = hash("jester_back_out")
M.MSG_TRANSITION_DONE = hash("jester_transition_done")
M.MSG_SAVE_STATE = hash("jester_save_state")
return M
