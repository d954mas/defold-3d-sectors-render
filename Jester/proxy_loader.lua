local HASHES = require "Jester.libs.hashes"
local M = {}
M.scenes = {}
function M.load(scene, co)
    assert(co, "co can't be nil")
    assert(not M.scenes[scene._url], " scene is loading now:" .. scene._name)
    M.scenes[tostring(scene._url)] = { scene = scene, co = co }
    msg.post("main:/jester_proxy_loader", HASHES.MSG_LOAD_PROXY, { url = scene._url})
end

function M.load_done(url)
    local data = M.scenes[tostring(url)]
    if data then
        M.scenes[tostring(url)] = nil
        local ok, res = coroutine.resume(data.co)
        if not ok then
            print(res)
        end
    end
end

function M.unload(scene)
    msg.post(scene._url, HASHES.MSG_UNLOAD)
end

return M