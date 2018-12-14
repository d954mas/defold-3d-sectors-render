local COMMON = require "libs.common"
local M = {}

function M.load(file)
	COMMON.i("load level:" .. file)
	local data = resource.load("/assets/levels/" .. file .. ".txt")
	native.load_level(data)
end

function M.unload()
	native.unload_level()
end

return M