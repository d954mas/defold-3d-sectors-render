local COMMON = require "libs.common"
local Script = COMMON.class("ScriptsContainer")

local function add_if_can(self,name,script)
	if script[name] then
		table.insert(self["scripts_" .. name],script)
	end
end


function Script:initialize()
	self.scripts_init = {}
	self.scripts_final = {}
	self.scripts_update = {}
	self.scripts_on_message = {}
	self.scripts_on_input = {}
	self.scripts_on_reload = {}
end


function Script:add_script(script)
	assert(script)
	add_if_can(self,"init",script)
	add_if_can(self,"final",script)
	add_if_can(self,"update",script)
	add_if_can(self,"on_message",script)
	add_if_can(self,"on_input",script)
	add_if_can(self,"on_reload",script)
end

function Script:init(go)
	for i=1, #self.scripts_init do
		self.scripts_init[i]:init(go)
	end
end

function Script:final(go)
	for i=1, #self.scripts_final do
		self.scripts_final[i]:final(go)
	end
end

function Script:update(go, dt)
	for i=1, #self.scripts_update do
		self.scripts_update[i]:update(go,dt)
	end
end

function Script:on_message(go, message_id, message, sender)
	for i=1, #self.scripts_on_message do
		self.scripts_on_message[i]:on_message(go,message_id,message,sender)
	end
end

function Script:on_input(go, action_id, action)
	for i=1, #self.scripts_on_input do
		self.scripts_on_input[i]:on_input(go,action_id,action)
	end
end

function Script:on_reload(go)
	for i=1, #self.scripts_on_reload do
		self.scripts_on_reload[i]:on_reload(go)
	end
end


return Script