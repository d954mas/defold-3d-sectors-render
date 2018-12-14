local CLASS = require "Jester.libs.middleclass"
local Stack = require "Jester.libs.stack"
local Scene = require "Jester.scene"
local LOG = require "Jester.libs.log"
local STATES = Scene.static.STATES
local TRANSITIONS = Scene.static.TRANSITIONS
local TAG = "SceneManager"

---@class JesterSceneManager
---@field stack SceneStack
local M = CLASS.class("SceneStack")



--region change state
local function check(scene, co)
    assert(scene, "scene can't be nil")
    assert(co, "co can't be nil")
end

local function unload(scene, co)
    check(scene, co)
    LOG.i("unload scene:" .. scene._name, TAG)
    scene:unload(co)
    scene._state = STATES.UNLOADED
end

local function load(scene, co)
    check(scene, co)
    LOG.i("start load scene:" .. scene._name, TAG)
    local start_loading_time = os.clock()
    scene:load(co)
    LOG.i("load scene:" .. scene._name .. " done", TAG)
    LOG.i("load time:" .. os.clock() - start_loading_time, TAG)
    scene._state = STATES.HIDE
end

local function pause(scene, co)
    check(scene, co)
    LOG.i("pause scene:" .. scene._name, TAG)
    scene:pause(co)
    scene._state = STATES.PAUSED
end

local function resume(scene, co)
    check(scene, co)
    LOG.i("resume scene:" .. scene._name, TAG)
    scene:resume(co)
    scene._state = STATES.RUNNING
end

local function hide(scene, co)
    check(scene, co)
    LOG.i("hide scene:" .. scene._name, TAG)
    scene:hide(co)
    scene._state = STATES.HIDE
end

local function show(scene, co, input)
    check(scene, co)
    LOG.i("show scene:" .. scene._name, TAG)
    scene:show(co, input)
    scene._state = STATES.PAUSED
end

local function transition(type, scene, co)
    assert(type, "type can't be nil")
    check(scene, co)
    LOG.i(type .. " scene:" .. scene._name, TAG)
    local state = scene._state
    scene._state = STATES.TRANSITION
    scene[type](scene, co)
    scene._state = state
end

function M:initialize(name)
    assert(name, "name can't be nil")
    ---@type JesterSceneStack
    self.stack = Stack()
    self.scenes = {}
    self.name = name
end

function M:register(scenes)
    assert(#self.scenes == 0, "register_scenes can be called only once")
    assert(scenes, "scenes can't be nil")
    assert(#scenes ~= 0, "scenes should have one or more scene")
    for _, scene in ipairs(scenes) do
        assert(not scene.__declaredMethods, "register instance not class(add ())")
        assert(scene._name, "scene name can't be nil")
        assert(not self.scenes[scene._name], "scene:" .. scene._name .. " already exist")
        self.scenes[scene._name] = scene
        scene._sm = self
    end
end

--COROUTUINES FUN
--show new scene, hide current scene
---@param current_scene JesterScene
---@param new_scene JesterScene
local function show_new_scene(self, current_scene, new_scene, new_scene_input, options, is_back)
    assert(self, "self can't be nil")
    assert(new_scene, "new_scene can't be nil")
    LOG.i("change scene from " .. (current_scene and current_scene._name or "nil") .. " to " .. new_scene._name)
    options = options or {}

    if new_scene == current_scene and not options.reload then
        LOG.i("scene:" .. current_scene._name .. " already on top")
        self.co = nil
        return
    end
    --try preload scene
    if new_scene._state == STATES.UNLOADED then
        load(new_scene, self.co)
    end

    if current_scene then
        LOG.i("release input for scene:" .. current_scene._name, TAG)
        current_scene:release_input()
        if not options.popup then
            transition(is_back and TRANSITIONS.BACK_OUT or TRANSITIONS.SHOW_OUT, current_scene, self.co)
        end
        if current_scene._state == STATES.RUNNING and not options.not_pause then
            pause(current_scene, self.co)
        end
        if current_scene._state == STATES.PAUSED and not options.popup then
            hide(current_scene, self.co)
        end
        if current_scene._state == STATES.HIDE  and not current_scene._do_not_unload then
            current_scene._state = STATES.UNLOADED
            unload(current_scene, self.co)
        end
    end

    local need_transition = false
    if new_scene._state == STATES.UNLOADED then
        load(new_scene, self.co)
    end
    if new_scene._state == STATES.HIDE then
        show(new_scene, self.co, new_scene_input)
        need_transition = true
    end
    if new_scene._state == STATES.PAUSED then
        resume(new_scene, self.co)
    end
    if need_transition then
        transition(is_back and TRANSITIONS.BACK_IN or TRANSITIONS.SHOW_IN, new_scene, self.co)
    end
    LOG.i("acquire input for scene:" .. new_scene._name, TAG)
    new_scene:acquire_input()
    self.co = nil
    LOG.i("scene changed", TAG)
end

function M:show(scene_name, input, options)
    assert(not self.co, "work in progress.Can't show new scene")
    local scene = self:get_scene_by_name(scene_name)
    LOG.i("show " .. scene_name)
    local current_scene = self.stack:pop()
    self.stack:push(scene)
    self.co = coroutine.create(show_new_scene)
    local ok, res = coroutine.resume(self.co, self, current_scene, scene, input, options)
    if not ok then
        LOG.e(res, TAG)
        self.co = nil
    end
end

function M:reload()
   self:show(self.stack:peek()._name,self.stack:peek()._input, {reload = true})
end

--region UTILS
function M:get_scene_by_name(name)
    assert(name, "name can't be nil")
    local scene = self.scenes[name]
    assert(scene, "unknown scene:" .. name)
    return scene
end
--endregion


--region AUTOCOMPLETE
---@class JesterSceneStackData
---@field scene JesterScene

--luacheck: push ignore 241
---@class JesterSceneStack:JesterStack
local StackClass = {}
---@return JesterScene
function StackClass:pop()
end

---@return JesterScene
function StackClass:peek(value)
end
--endregion
--luacheck: pop
return M