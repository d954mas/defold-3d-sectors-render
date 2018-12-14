--USE IT TO CALL SOME EVENTS IN CONTEXT OF GO
local COMMON = require "libs.common"
local SubscriptionStorage = require "libs.subscription_storage"

local ContextSubscription = COMMON.class("ContextSubscription")

function ContextSubscription:initialize()
	self.events = {} --received events
	self.observable_events_map = {}
end

---@param observable Observable
---@param event any event_id
---@param fun function call when event happened (go_self, model,data,event_id)
---@param multiple boolean true keep all, else keep only last
function ContextSubscription:add(observable, event, fun, multiple)
	assert(observable)
	assert(event)
	assert(fun)

	--can't have multiple subscription to same observable with same event
	self.observable_events_map[observable] = self.observable_events_map[observable] or {}
	local observable_events_map = self.observable_events_map[observable]
	assert(not observable_events_map[event],"duplicate subscription")

	local subscription_fun
	if multiple then
		subscription_fun = function(model,data,event)
			table.insert(self.events,{model = model, data = data, event = event})
		end
	else
		subscription_fun =  function(model,data,event)
			local t = self.observable_events_map[model][event]
			if t.received then return end
			t.received = true
			table.insert(self.events,{model = model, data = data, event = event, is_single = true})
		end
	end
	local subscription = observable:register_observer(event,subscription_fun)
	observable_events_map[event] = {fun=fun, received = false, subscription = subscription}--received - flag for single events
end

function ContextSubscription:act(go_self)
	for i=1, #self.events do
		local event = self.events[i]
		local t = self.observable_events_map[event.model][event.event]
		t.fun(go_self,event.model,event.data, event.event)
		t.received = false
		self.events[i] = nil
	end
end

function ContextSubscription:clear_events()
	self.events = {}
end

function ContextSubscription:unsubscribe()
	for _, observable in pairs(self.observable_events_map) do
		for _,event in pairs(observable) do
			event.subscription:unsubscribe()
		end
	end
	self.observable_events_map = {}
	self.events = {}
end

function ContextSubscription:unsubscribe_single(observable, event)
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

function ContextSubscription:unsubscribe()
	for _, observable in pairs(self.observable_events_map) do
		for _,event in pairs(observable) do
			event.subscription:unsubscribe()
		end
	end
	self.observable_events_map = {}
	self.events = {}
end

return ContextSubscription