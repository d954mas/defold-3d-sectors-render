local COMMON = require "libs.common"
local Script = COMMON.class("BaseScript")

function Script:init(go)
end

function Script:final(go)
end

function Script:update(go, dt)
end

function Script:on_message(go, message_id, message, sender)
end

function Script:on_input(go, action_id, action)
end

function Script:on_reload(go)
end


return Script