local SceneManager = require "Jester.scene_stack"

local M = {}
M.SCENE_MANAGERS = {}
M.MANAGER = SceneManager("main")

function M.register(scenes)
	M.MANAGER:register(scenes)
end

function M.register_scene_manager(sm)
	assert(sm, "scene manager can't be nil")
	assert(not M.SCENE_MANAGERS[sm.name], "scene with name:" .. sm.name .. " already registered")
	M.SCENE_MANAGERS[sm.name] = sm
end

function M.show(name, input, options)
	M.MANAGER:show(name, input, options)
end

function M.popup(name, input, options)
	M.MANAGER:popup(name, input, options)
end

function M.back()
	M.MANAGER:back()
end

function M.reload()
	M.MANAGER:reload()
end

function M.get_scene_by_name(name)
	return M.MANAGER:get_scene_by_name(name)
end

M.register_scene_manager(M.MANAGER)

return M