local COMMON = require "libs.common"
local Observable = require "libs.observable_mixin"

---@class PlayerNew:Observable
local M = COMMON.class("Player")
M:include(Observable)
local EVENTS = {
}

function M:initialize()
	self.EVENTS = EVENTS
	self:set_observable_events(self.EVENTS)
	self.e = Entity.new_unit()
end

function M:dispose()
	self.e:destroy()
end

--region getter
function M:get_angle()

end

function M:get_position()
	local x,y,z = self.e:get_position()
	return x,y,z
end

function M:get_velocity()
	local x,y,z = self.e:get_velocity()
	return x,y,z
end
--endregion


function M:set_position(x,y,z)
	self.e:set_position(x,y,z)
end

function M:set_sector(n)
	self.e:set_sector(n)
end

function M:set_movement_speed(speed)
	self.e:set_movement_speed(speed)
end


function M:set_velocity(x,y,z)
	self.e:set_velocity(x,y,z)
end

function M:set_angle(angle)
	self.e:set_angle(angle)
end

---set z position to floor position + HeadMargin
function M:align_z()

end


return M