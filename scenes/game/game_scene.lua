local ProxyScene = require "Jester.proxy_scene"
local Scene = ProxyScene:subclass("LogoScene")
local COMMON = require "libs.common"
local SM = require "Jester.scene_stack"
local JESTER = require "Jester.jester"
local WORLD = require "world.world"
local Subscription = require "libs.context_subscription"
local RenderHelper = require "native_render.helper"


function Scene:init_input()
    COMMON.input_acquire()
    self.input_receiver = COMMON.INPUT()
    self.input_receiver:add(hash("lock_mouse"),function() lock_mouse.lock_mouse() end, true)
    self.input_receiver:add(hash("unlock_mouse"),function() lock_mouse.unlock_mouse() end, true)
end
--endregion

function Scene:initialize()
    ProxyScene.initialize(self, "GameScene", "/game#proxy", "game:/scene_controller")
    self.msg_receiver = COMMON.MSG()
end

function Scene:on_show(input)

end

function Scene:on_hide()
    window.set_listener(function() end)
end

function Scene:init(go_self)
    RenderHelper.init()
    self:init_input()
    self.subscription = Subscription()
    lock_mouse.lock_mouse()
end

function Scene:final(go_self)
    self.subscription:unsubscribe()
    COMMON.input_release()
    WORLD:save()
    WORLD:dispose()
    lock_mouse.unlock_mouse();
end

function Scene:update(go_self, dt)
    self.subscription:act()
    WORLD:update(dt)
end

function Scene:show_out(co)

end

function Scene:on_message(go_self, message_id, message, sender)
    self.msg_receiver:on_message(self, message_id, message, sender)
end

function Scene:on_input(go_self, action_id, action, sender)
   self.input_receiver:on_input(go_self,action_id,action,sender)
end

--region MSG_RECEIVERS

function Scene:load(co)
    ProxyScene.load(self,co)
    WORLD:load(nil,"map_triangle")
end



--endregion
return Scene