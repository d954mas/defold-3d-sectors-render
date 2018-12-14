local ProxyScene = require "Jester.proxy_scene"
local SM = require "Jester.jester"
local Scene = ProxyScene:subclass("LogoScene")

--- Constructor
-- @param name Name of scene.Must be unique
function Scene:initialize()
    ProxyScene.initialize(self, "LogoScene", "/logo#proxy", "logo:/scene_controller")
end

function Scene:on_show(input)
    SM.show("GameScene")
end

function Scene:final(go_self)
end

function Scene:update(go_self, dt)
end

function Scene:show_out(co)
end
return Scene