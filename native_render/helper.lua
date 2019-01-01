local COMMON = require "libs.common"
local MAP = require("world.map")
local M = {}

M.WIDTH = 960/2
M.HEIGHT = 540/2
M.BUFFER = buffer.create(M.WIDTH * M.HEIGHT, { {name=hash("rgb"), type=buffer.VALUE_TYPE_UINT8, count=3} } )


function M.init()
	native.render_set_buffer(M.WIDTH, M.HEIGHT, M.BUFFER)
end


function M.update_texture(resource_path)
	local header = {width = M.WIDTH, height =M.HEIGHT, type = resource.TEXTURE_TYPE_2D, format = resource.TEXTURE_FORMAT_RGB, num_mip_maps = 0}
	resource.set_texture(resource_path, header, M.BUFFER)
end



return M