local COMMON = require "libs.common"
local Script = COMMON.class("PlayerInputScript")
local World = require("world.world")
local MOUSE_SCALE = 0.3
function Script:init(go)
	self.input_receiver = COMMON.INPUT()
	self.go_self = go
	self.input_receiver:add(COMMON.HASHES.INPUT_UP, function(self, action_id,action)
		if action.pressed then
			World.player.velocity.x = 1
		elseif action.released then
			World.player.velocity.x = 0
		end
	end)
	self.input_receiver:add(COMMON.HASHES.INPUT_DOWN, function(self, action_id,action)
		if action.pressed then
			World.player.velocity.x = -1
		elseif action.released then
			World.player.velocity.x = 0
		end
	end)

	self.input_receiver:add(COMMON.HASHES.INPUT_LEFT, function(self, action_id,action)
		if action.pressed then
			World.player.velocity.y = -1
		elseif action.released then
			World.player.velocity.y = 0
		end
	end)

	self.input_receiver:add(COMMON.HASHES.INPUT_RIGHT, function(self, action_id,action)
		if action.pressed then
			World.player.velocity.y = 1
		elseif action.released then
			World.player.velocity.y = 0
		end
	end)

	self.input_receiver:add_mouse(function()
		local move=lock_mouse.update_cursor()
		if(move)then
			World.player.angle =World.player.angle +  move[1] * 0.03 *MOUSE_SCALE;
			local yaw  = COMMON.LUME.clamp(move[2]*0.05*MOUSE_SCALE, -5, 5);
			World.player.yaw = yaw - World.player.velocity.z*0.5;
		end
	end)

end

function Script:final(go)
end

function Script:update(go, dt)
end

function Script:on_input(go, action_id, action)
	self.input_receiver:on_input(self,action_id,action)
end

return Script