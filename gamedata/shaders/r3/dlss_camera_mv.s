function normal(shader, t_base, t_second, t_detail)
    shader:begin("stub_screen_space", "dlss_camera_mv")
        :zb(false, false)
        :blend(false, blend.one, blend.zero)
    shader:dx10texture("s_scene_depth", "$user$dlss_scene_depth")
    shader:dx10texture("s_motion_vectors", "$user$ssfx_motion_vectors")
    shader:dx10texture("s_scope_mask", "$user$dlss_scope_mask")
end
