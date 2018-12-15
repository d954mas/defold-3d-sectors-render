local COMMON = require "libs.common"
local Observable = require "libs.observable_mixin"
local MultipleSubscription = require "libs.multiple_subscription"
local RenderHelper = require "native_render.helper"

---@class Player:Observable
local M = COMMON.class("Player")
M:include(Observable)
local EVENTS = {
}

function M:initialize()
	self.EVENTS = EVENTS
	self.subscription = MultipleSubscription()
	self:set_observable_events(self.EVENTS)
	self.position = vmath.vector3()
	self.velocity = vmath.vector3();
	self.speed =3
	self.angle = 0;
	self.yaw = 0;
end

function M:update_position(dt)
	self.position.x, self.position.y, self.position.z = native:get_player_pos()

	if(vmath.length(self.velocity)>0) then
		local move_step = vmath.normalize(self.velocity) * self.speed * dt
		self.position = self.position + vmath.rotate(vmath.quat_rotation_z(self.angle),move_step)
	end

	native.move_player(self.position.x, self.position.y)
end

	

function M:update(dt)
	native.set_player_yaw(self.yaw);
	native.set_player_angle(self.angle);
	self:update_position(dt)
end


return M