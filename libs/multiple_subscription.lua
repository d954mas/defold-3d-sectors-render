--USE IT TO CALL SOME EVENTS IN CONTEXT OF GO
local COMMON = require "libs.common"

local MultipleSubscription = COMMON.class("MultipleSubscription")

function MultipleSubscription:initialize()
	self.observable_events_map = {}
end

---@param observable Observable
---@param event any event_id
---@param fun function call when event happened (go_self, model,data,event_id)
function MultipleSubscription:add(observable, event, fun)
	assert(observable)
	assert(event)
	assert(fun)

	--can't have multiple subscription to same observable with same event
	self.observable_events_map[observable] = self.observable_events_map[observable] or {}
	local observable_events_map = self.observable_events_map[observable]
	assert(not observable_events_map[event],"duplicate subscription")

	local subscription = observable:register_observer(event,fun)
	observable_events_map[event] = {fun=fun, subscription = subscription}
end

function MultipleSubscription:unsubscribe_single(observable, event)
	assert(observable)
	local map = self.observable_events_map[observable]
	if map then
		if event then
			self.observable_events_map[event].subscription:unsubscribe()
			self.observable_events_map[event] = nil
		else
			for _,event in pairs(map) do
				event.subscription:unsubscribe()
			end
			self.observable_events_map[observable] = nil
		end
	end
end


function MultipleSubscription:unsubscribe()
	for _, observable in pairs(self.observable_events_map) do
		for _,event in pairs(observable) do
			event.subscription:unsubscribe()
		end
	end
	self.observable_events_map = {}
end

return MultipleSubscription