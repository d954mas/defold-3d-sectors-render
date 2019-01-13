local COMMON = require "libs.common"

---@class Map:Observable
local Map = {}
Map.__index = Map -- failed table lookups on the instances should fallback to the class table, to get methods

function Map:dispose()
	native.map.clear()
end

local function fixed_array_recursive(t)
	for i=1,#t do
		t[i-1]=t[i]
	end
	t[#t] = nil
	for k,v in pairs(t)do
		if type(v) =="table" then
			fixed_array_recursive(v)
		end
	end
end



local M = {}
function M.load(map_name)
	local json_str = sys.load_resource("/assets/levels/" .. map_name .. ".json")
	local map = json.decode(json_str)
	fixed_array_recursive(map)
	native.map.clear()
	for i=0, #map.vertices,1 do
		v = map.vertices[i]
		native.map.vertex_add(v[0],v[1])
	end
	for i=0, #map.sectors,1 do
		s = map.sectors[i]
		native.map.sector_create(s.floor_h, s.ceil_h)
		local len_vertices = #s.vertices+1
		for j=0, len_vertices-1,1 do
			local vertex_id = (j+len_vertices-1)%len_vertices
			local v = s.vertices[vertex_id].v
			local n = s.vertices[j].n
			native.map.sector_vertex_add(v, n)
		end
		native.map.sector_vertex_add(s.vertices[len_vertices-1].v, -1)
	end

	local player = map.player
	native.map.check()
	--fixed_arrays(map.vertices)
	--fixed_arrays(map.sectors)
	--clear prev map

	local self = setmetatable(map, Map)
	return self
end

return M