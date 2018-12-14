local M = {}

function M.init()
    M.stencil_mask_enabled = false
    M.mask_pred = render.predicate({ "jester_screenfade" })
    M.screen_pred = render.predicate({"jester_screen"})
    M.clear_color = vmath.vector4(0, 0, 0, 1)
    M.clear_table = {[render.BUFFER_COLOR_BIT] = M.clear_color, [render.BUFFER_DEPTH_BIT] = 1, [render.BUFFER_STENCIL_BIT] = 0}
end

function M.render_to_texture_begin()
    if M.prepare_texture then
        print("rendered to texture")
        if not M.render_target then
            M.init_render_target()
        end
        render.enable_render_target(M.render_target)
        render.clear(M.clear_table)
    end
end

function M.init_render_target()
    local color_params = { format = render.FORMAT_RGBA,
                           width = render.get_window_width(),
                           height = render.get_window_height(),
                           min_filter = render.FILTER_LINEAR,
                           mag_filter = render.FILTER_LINEAR,
                           u_wrap = render.WRAP_CLAMP_TO_EDGE,
                           v_wrap = render.WRAP_CLAMP_TO_EDGE }
    local depth_params = { format = render.FORMAT_DEPTH,
                           width = render.get_window_width(),
                           height = render.get_window_height(),
                           u_wrap = render.WRAP_CLAMP_TO_EDGE,
                           v_wrap = render.WRAP_CLAMP_TO_EDGE }
    M.render_target = render.render_target("stencil_render_screen",{ [render.BUFFER_COLOR_BIT] = color_params, [render.BUFFER_DEPTH_BIT] = depth_params })
end

function M.render_to_texture_end()
    if M.prepare_texture then
        M.prepare_texture = false
        render.disable_render_target(M.render_target)
        M.render()
    end
end

function M.render()
    if M.stencil_mask_enabled then
        if M.use_texture and not M.prepare_texture then
            render.set_view(vmath.matrix4())
            render.set_projection( vmath.matrix4_orthographic(-1, 1, -1, 1, -1, 1))
            render.enable_texture(0, M.render_target, render.BUFFER_COLOR_BIT)
            render.draw(M.screen_pred)
            render.disable_texture(0, M.render_target)
        end
        -- Using the simple trick from Sven's post here: https://forum.defold.com/t/making-a-flashlight/286/3
        -- 1. We need to disable color drawing while rendering the mask
        render.set_color_mask(false, false, false, false)
        -- 2. Enable stencil test and setup stencil mask parameters
        render.enable_state(render.STATE_STENCIL_TEST)
        render.set_stencil_func(render.COMPARE_FUNC_ALWAYS, 1, 255)
        render.set_stencil_op(render.STENCIL_OP_KEEP, render.STENCIL_OP_KEEP, render.STENCIL_OP_REPLACE)
        render.set_stencil_mask(255)
        -- 3. Draw the mask
        render.set_view(vmath.matrix4())
        render.set_projection(vmath.matrix4())
        render.draw(M.mask_pred)
        -- 4. Update the stencil function to only let pixel pass that are equal to the mask result
        render.set_stencil_func(render.COMPARE_FUNC_EQUAL, 1, 255)

        -- 5. Re-enable color drawing
        render.set_color_mask(true, true, true, true)

        -- 6. Continue as rendering usual! :)
        render.enable_state(render.STATE_BLEND)
        render.set_blend_func(render.BLEND_SRC_ALPHA, render.BLEND_ONE_MINUS_SRC_ALPHA)
        render.disable_state(render.STATE_CULL_FACE)
    end
end

function M.update(dt)
    if M.need_update then
        M.time = M.time + dt
        if M.time > M.max_time then
            M.need_update = false
            M.time = 0
            M.max_time = 0
            M.revert = nil
            if not M.no_hide then M.stencil_mask_enabled = false end
            return
        end
        local offset =  M.time / M.max_time
        if M.revert then
            offset = 1 - offset
        end
        model.set_constant("main:/jester_stencil#model", "offset", vmath.vector4(offset, 1, 1, 1))
    end
end

function M.show(time, revert, use_texture, no_hide)
    M.stencil_mask_enabled = true
    M.need_update = true
    M.max_time = time
    M.time = 0
    M.revert = revert
    M.use_texture = use_texture
    M.prepare_texture = use_texture
end

function M.hide()
    M.stencil_mask_enabled = false
end

function M.on_message(self, message_id, message)
    if message_id == hash("disable_stencil_mask") then
        M.stencil_mask_enabled = false
    elseif message_id == hash("enable_stencil_mask") then
        M.stencil_mask_enabled = true
    end
end

return M