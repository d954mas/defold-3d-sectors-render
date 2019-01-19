local COMMON = require "libs.common"
local Observable = require "libs.observable_mixin"
local MultipleSubscription = require "libs.multiple_subscription"
local MAP = require "world.map"
local Player = require "world.player"

---@class World:Observable
local M = COMMON.class("World")
M:include(Observable)
local EVENTS = {
}

function M:initialize()
	self.EVENTS = EVENTS
	self.subscription = MultipleSubscription()
	self:set_observable_events(self.EVENTS)
end

function M:update(dt, no_save)
	native.world.update(dt)
	self:draw()
end

function M:draw()
	native.render.draw_screen(self.player.e)
end


function M:save()

end

function M:dispose()
	self.map:dispose()
	self.map = nil
	self.player:dispose()
	self.player = nil
end

function M:load(file)
	file = file or "map_clear"
	COMMON.i("load level:" .. file)
	self.map = MAP.load(file)
	--region init player
	self.player= Player()
	self.player:set_sector(self.map.player.sector)
	self.player:set_position(self.map.player.position.x,self.map.player.position.y,0)
	self.player:set_movement_speed(20)
	self.player:align_z()
	self.player:set_angle(math.pi/2)
	--endregion
end

return M()