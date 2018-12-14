local HASH_NIL = hash("nil")
local HASH_ACQUIRE = hash("acquire_input_focus")
local HASH_RELEASE = hash("release_input_focus")
local M = {}
M.__index = M

local function ensure_hash(string_or_hash)
	return type(string_or_hash) == "string" and hash(string_or_hash) or string_or_hash
end

function M.new()
	local self = setmetatable({}, M)
	self.action_funs = {}
	return self
end

function M:add_mouse(fun)
	assert(fun,"function can't be null")
	local action_id = HASH_NIL
	assert(not self.action_funs[action_id], action_id .. " already used")
	self.action_funs[action_id] = {fun = fun}
end	


function M:add(action_id, fun, is_pressed, is_repeated)
	assert(action_id,"action_id can't be null")
	assert(fun,"function can't be null")
	action_id = ensure_hash(action_id)
	assert(not self.action_funs[action_id], action_id .. " already used")
	self.action_funs[action_id] = {fun = fun, is_pressed = is_pressed, is_repeated = is_repeated}
end

function M:on_input(go_self, action_id, action)
	action_id = action_id or HASH_NIL
	local fun = self.action_funs[action_id]
	local f_pressed, f_repeated = fun.is_pressed , fun.is_repeated
	if fun then
		if not (f_pressed or f_repeated) or  (f_pressed and action.pressed) or (f_repeated and action.repeated)  then
			fun.fun(go_self, action_id, action)	
			return true
		end	
	end
	return false
end

function M.acquire(url)
	msg.post(url or ".", HASH_ACQUIRE)
end

function M.release(url)
	msg.post(url or ".", HASH_RELEASE)
end

return M
