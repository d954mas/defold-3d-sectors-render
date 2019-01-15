local COMMON = require "libs.common"
local Script = COMMON.class("PlayerInputScript")
local World = require("world.world")
local MOUSE_SCALE = 0.3


function Script:update_velocity()
	self.velocity.x, self.velocity.y = 0,0
	if self.up then
		self.velocity.y = self.velocity.y + 1
	end
	if self.down then
		self.velocity.y = self.velocity.y - 1
	end
	if self.left then
		self.velocity.x= self.velocity.x + 1
	end
	if self.right then
		self.velocity.x = self.velocity.x - 1
	end
	if vmath.length(self.velocity)~= 0 then
		self.velocity = vmath.normalize(self.velocity)
		World.player:set_velocity(self.velocity.x,self.velocity.y,nil)
	else
		World.player:set_velocity(0,0,nil)
	end

end

function Script:init(go)
	self.velocity = vmath.vector3()
	self.input_receiver = COMMON.INPUT()
	self.go_self = go
	self.right, self.left, self.up, self.down = nil, nil, nil, nil
	self.input_receiver:add(COMMON.HASHES.INPUT_UP, function(self, action_id,action)
		if action.pressed then
			self.up = true
			self:update_velocity()
		elseif action.released then
			self.up = nil
			self:update_velocity()
		end
	end)
	self.input_receiver:add(COMMON.HASHES.INPUT_DOWN, function(self, action_id,action)
		if action.pressed then
			self.down = true
			self:update_velocity()
		elseif action.released then
			self.down = nil
			self:update_velocity()
		end
	end)

	self.input_receiver:add(COMMON.HASHES.INPUT_LEFT, function(self, action_id,action)
		if action.pressed then
			self.left = true
			self:update_velocity()
		elseif action.released then
			self.left = nil
			self:update_velocity()
		end
	end)

	self.input_receiver:add(COMMON.HASHES.INPUT_RIGHT, function(self, action_id,action)
		if action.pressed then
			self.right = true
			self:update_velocity()
		elseif action.released then
			self.right = nil
			self:update_velocity()
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
		World.player:increase_angle(-action.screen_dx * 0.03 *MOUSE_SCALE)
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