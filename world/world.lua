local COMMON = require "libs.common"
local Observable = require "libs.observable_mixin"
local MultipleSubscription = require "libs.multiple_subscription"
local MAP = require "world.map"
local Player = require "world.player"
local PlayerNew = require "world.player_new"

---@class World:Observable
local M = COMMON.class("World")
M:include(Observable)
local EVENTS = {
}

function M:initialize()
	self.EVENTS = EVENTS
	self.subscription = MultipleSubscription()
	self:set_observable_events(self.EVENTS)
	self.player = Player()
end



function M:update(dt, no_save)
	self.player:update(dt)
	native.world_update(dt)
	self:draw()
end

function M:draw()
	native.draw_screen(self.player_new.e)
end


function M:save()

end

function M:dispose()
	self.map:dispose()
	self.map = nil
	self.player_new:dispose()
	self.player_new = nil
end

function M:load(file)
	file = file or "map_clear"
	COMMON.i("load level:" .. file)
	self.map = MAP.load(file)
	--region init player
	self.player_new = PlayerNew()
	self.player_new:set_sector(self.map.player.sector)
	self.player_new:set_position(self.map.player.position.x,self.map.player.position.y,0)
	self.player_new:align_z()
	self.player_new:set_angle(0)
	--endregion
end

return M()