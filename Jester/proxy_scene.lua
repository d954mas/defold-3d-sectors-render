local HASHES = require "Jester.libs.hashes"
local BaseScene = require "Jester.scene"
local PROXY_LOADER = require "Jester.proxy_loader"
local Scene = BaseScene:subclass("ProxyScene")
--- Constructor
-- @param name Name of scene.Must be unique
function Scene:initialize(name, url, controller_url)
	BaseScene.initialize(self)
	self._name = name
	self._url = msg.url(url)
	self._controller_url = msg.url(controller_url)
end

function Scene:show(co, input)
	BaseScene.show(self, co, input)
	msg.post(self._url, HASHES.MSG_ENABLE)
	if self.__loaded then
		msg.post(self._controller_url, HASHES.MSG_JESTER_INIT, {scene_name = self._name, sm_name = self._sm.name})
		self.__loaded = nil
	end
	msg.post(self._controller_url, HASHES.MSG_SHOW)
end

function Scene:hide(co)
	msg.post(self._url, HASHES.MSG_DISABLE)
end

function Scene:acquire_input()
	msg.post(self._url, HASHES.INPUT_ACQUIRE_FOCUS)
end

function Scene:release_input()
	msg.post(self._url, HASHES.INPUT_RELEASE_FOCUS)
end

function Scene:load(co)
	PROXY_LOADER.load(self, co)
	coroutine.yield()
	self.__loaded = true
end

function Scene:unload()
	PROXY_LOADER.unload(self)
end

function Scene:pause()
	msg.post(self.url, "set_time_step", {factor = 0, mode = 0})
end
function Scene:resume()
	msg.post(self.url, "set_time_step", {factor = 1, mode = 0})
end

function Scene:show_in(co)
end

function Scene:show_out(co)
end

function Scene:back_in(co)
end

function Scene:back_out(co)
end

return Scene