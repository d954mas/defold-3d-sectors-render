local COMMON = require "libs.common"
local Observable = require "libs.observable_mixin"
local MultipleSubscription = require "libs.multiple_subscription"
local RenderHelper = require "native_render.helper"

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

end


function M:save()

end

function M:unload()
	RenderHelper.unload()
end

function M:load()
	RenderHelper.init()
	RenderHelper.load("map_clear")
end

return M()