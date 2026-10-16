#pragma once

class CActor;
class IKinematicsAnimated;
class IKinematics;
struct IDSGraphManager;

// Local first-person pivot; does not move the physics actor.
extern BOOL g_body_pivot;
extern BOOL g_body_natural_turn;
extern float g_body_eye_forward;
extern BOOL g_body_eye_auto;
float body_pivot_eye_distance(CActor* actor);
bool body_pivot_active(CActor* actor);

class player_legs_controller
{
public:
    void    update(CActor* actor, bool isShadowPass = false);
    void    render(IDSGraphManager* DM, bool isShadowPass = false);
    void    destroy();
    void    record_camera_collision(const Fvector& requested, const Fvector& resolved);

    bool    is_active() const { return m_model != nullptr; }
    IKinematics* model() const { return m_model; }

private:
    Fvector m_camera_collision = {0.f, 0.f, 0.f};
    u32 m_collision_frame = u32(-1);
    u32 m_retreat_frame = u32(-1);
    float m_collision_retreat = 0.f;
    IKinematics* m_model = nullptr;
    IKinematics* m_shadow_model = nullptr;
    shared_str              m_visual_name;
    shared_str              m_last_outfit_sect;
    shared_str              m_last_model;
    std::optional<float> m_fwd_offset = std::nullopt;
    float                   m_y_offset = 0.f;
    Fmatrix                 m_legs_transform;
    bool                    m_first_frame = true;
    xr_vector<Fmatrix>      m_prev_bones;
    bool                    m_has_prev = false;
    float                   m_blend_speed = 8.f;
    bool                    m_logged = false;
    bool is_costume_affected_bone(LPCSTR bone_name) const;
    bool should_hide_bone(LPCSTR bone_name) const;
    bool is_spine_bone(LPCSTR bone_name) const;
    void remove_camera_pitch(Fmatrix& bone_transform);

    bool    resolve_config(CActor* actor, shared_str& sect, shared_str& model);
    bool    ensure_model(const shared_str& sect, const shared_str& model);
    void    copy_bones_from_actor(CActor* actor);
    void    warn_once(const char* fmt, ...);
    bool is_keep_bind_bone(LPCSTR bone_name) const;
};
