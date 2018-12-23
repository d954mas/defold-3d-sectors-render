local COMMON = require "libs.common"
local Script = COMMON.class("PlayerInputScript")
local World = require("world.world")
local MOUSE_SCALE = 0.3
function Script:init(go)
	self.input_receiver = COMMON.INPUT()
	self.go_self = go
	self.input_receiver:add(COMMON.HASHES.INPUT_UP, function(self, action_id,action)
		if action.pressed then
			World.player:set_velocity(1,nil,nil)
		elseif action.released then
			World.player:set_velocity(0,nil,nil)
		end
	end)
	self.input_receiver:add(COMMON.HASHES.INPUT_DOWN, function(self, action_id,action)
		if action.pressed then
			World.player:set_velocity(-1,nil,nil)
		elseif action.released then
			World.player:set_velocity(0,nil,nil)
		end
	end)

	self.input_receiver:add(COMMON.HASHES.INPUT_LEFT, function(self, action_id,action)
		if action.pressed then
			World.player:set_velocity(nil,-1,nil)
		elseif action.released then
			World.player:set_velocity(nil,0,nil)
		end
	end)

	self.input_receiver:add(COMMON.HASHES.INPUT_RIGHT, function(self, action_id,action)
		if action.pressed then
			World.player:set_velocity(nil,1,nil)
		elseif action.released then
			World.player:set_velocity(nil,0,nil)
		end
	end)

	self.input_receiver:add(COMMON.HASHES.INPUT_JUMP, function(self, action_id,action)
		if action.pressed and not World.player:is_falling() then
			World.player:set_velocity(nil,nil,1)
		end
	end)

	self.input_receiver:add_mouse(function(self, action_id,action)
		local move=lock_mouse.update_cursor()
		if(move)then
			action.screen_dx, action.screen_dy = move[1], move[2]
		end
		World.player:increase_angle(action.screen_dx * 0.03 *MOUSE_SCALE)
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