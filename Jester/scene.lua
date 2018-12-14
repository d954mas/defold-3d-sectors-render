--BASE SCENE MODULE.
local CLASS = require 'Jester.libs.middleclass'

---@class JesterScene
---@field _input
---@field _saved
---@field _state
---@field _name
local Scene = CLASS.class('Scene')
Scene.static.STATES = {
    UNLOADED = 0,
    HIDE = 1, --scene is loaded.But not showing on screen
    PAUSED = 2, --scene is showing.But update not called.
    RUNNING = 3, --scene is running
    TRANSITION = 4
}

Scene.static.TRANSITIONS = {
    SHOW_IN =  "show_in" ,
    SHOW_OUT = "show_out",
    BACK_IN =  "back_in" ,
    BACK_OUT = "back_out"
}

--- Constructor
-- @param name Name of scene.Must be unique
function Scene:initialize(name)
    self._name = name
    self._state = Scene.STATES.UNLOADED
end


--region BASE

function Scene:load(co)
end

function Scene:unload()
end

function Scene:hide(co)
end
function Scene:show(co, input)
    self._input = input
end

function Scene:pause(co)
end
function Scene:resume(co)
end

function Scene:acquire_input(co)
end
function Scene:release_input(co)
end
--endregion

--region ON
function Scene:on_hide()
end
function Scene:on_show(input)
end

---@return table state
function Scene:on_save_state()
end

function Scene:on_pause()
end
function Scene:on_resume()
end
--endregion

--region GO
function Scene:init(go_self) end

function Scene:final(go_self)
end

function Scene:update(go_self, dt)
end

function Scene:on_message(go_self, message_id, message, sender)
end

function Scene:on_input(go_self, action_id, action)
end

function Scene:on_reload(go_self)
end
--endregion

--region TRANSITIONS
function Scene:show_in(co)
end
function Scene:show_out(co)
end
function Scene:back_in(co)
end
function Scene:back_out(co)
end

function Scene:show_transition(type, co)
    assert(type, "type can't be nil")
    assert(co, "co can't be nil")
    self[type](self, co)
end

--endregion

return Scene