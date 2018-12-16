local COMMON = require "libs.common"

---@class Map:Observable
local Map = COMMON.class("Map")
local MyClass = {} -- the table representing the class, which will double as the metatable for the instances
MyClass.__index = MyClass -- failed table lookups on the instances should fallback to the class table, to get methods

--arrays start from 0.
local function fixed_arrays(array)
	for i=1,#array do
		array[i-1]=i
	end
end

local M = {}
function M.load(map_name)
	local json_str = sys.load_resource("/assets/levels/" .. map_name .. ".json")
	local map = json.decode(json_str)
	fixed_arrays(map.vertices)
	fixed_arrays(map.sectors)
	--clear prev map

	local self = setmetatable(map, Map)
	return self
end

return M