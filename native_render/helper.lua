local COMMON = require "libs.common"
local M = {}

M.WIDTH = 608
M.HEIGHT = 480
M.BUFFER = buffer.create(M.WIDTH * M.HEIGHT, { {name=hash("rgb"), type=buffer.VALUE_TYPE_UINT8, count=3} } )


function M.init()
	native.set_buffer(M.WIDTH, M.HEIGHT, M.BUFFER)
end

function M.dispose()

end

function M.update_texture(resource_path)
--	self.resource_path = go.get("#model", "texture0")
	native.draw_screen()
	local header = {width = M.WIDTH, height =M.HEIGHT, type = resource.TEXTURE_TYPE_2D, format = resource.TEXTURE_FORMAT_RGB, num_mip_maps = 0}
	resource.set_texture(resource_path, header, M.BUFFER)
end

function M.load(file)
	COMMON.i("load level:" .. file)
	local data = resource.load("/assets/levels/" .. file .. ".txt")
	native.load_level(data)
end

function M.unload()
	native.unload_level()
end

return M