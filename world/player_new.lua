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
	local x,y,z = self.e:get_position()
	print("pos:" .. x)
end

function M:dispose()
	self.e:destroy()
end

--region getter
function M:get_angle()

end
--endregion


function M:set_position(x,y,z)
	self.e:set_position(x,y,z)
end

function M:set_sector(n)
	self.e:set_sector(n)
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