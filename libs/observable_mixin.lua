local LUME = require "libs.lume"
local COMMON = require "libs.common"
---@class ObservableSubscription
local Subscription = {}
function Subscription:unsubscribe() end
local total_subscription = 0
---@class Observable
local M = {
}

M.static = {
	get_total_subscriptions = function() return total_subscription end
}


function M:set_observable_events(events)
	assert(events, "events can't be nil")
	self._observable_events = events
	self._observable_events_observers={}
	for k,v in pairs(self._observable_events) do
		self._observable_events_observers[v] = {}
	end
end

---@return ObservableSubscription
function M:register_observer(event, fun)
	assert(event, "events can't be nil")
	assert(fun, "fun can't be nil")
	local subscription = {}
	--subscription is weak ref
	local ref = COMMON.weakref(self)
	subscription.unsubscribe = function()
		local self = ref()
		if self then self:remove_observer(subscription) end
	end

	assert(self._observable_events_observers[event], "unknown event:" .. event)
	table.insert(self._observable_events_observers[event], {fun = fun, subscription = subscription})
	total_subscription = total_subscription + 1
	return subscription
end


function M:remove_observer(subscription)
	assert(subscription, "url can't be nil")
	for i,subscriptions in pairs(self._observable_events_observers) do
		for i, v in ipairs(subscriptions) do
			if v.subscription == subscription then
				total_subscription = total_subscription - 1
				table.remove(subscriptions, i)
				break
			end
		end
	end
end

function M:observable_notify(event, data)
	assert(event, "event can't be nil")
	local event_observables=self._observable_events_observers[event]
	for i=1,#event_observables do
		event_observables[i].fun(self,data,event)
	end
end

function M:observable_clear()
	for i,v in pairs(self._observable_events_observers) do
		LUME.clearp(v)
	end
end


return M