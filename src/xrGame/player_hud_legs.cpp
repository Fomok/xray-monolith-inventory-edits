#include "stdafx.h"
#include "player_hud_legs.h"
#include "body_pivot_turn.h"
#include "player_hud.h"
#include "actor.h"
#include "inventory_item.h"
#include "Inventory.h"
#include "../Include/xrRender/Kinematics.h"
#include "../Include/xrRender/KinematicsAnimated.h"
#include "../xrEngine/FDemoRecord.h"
#include "../xrEngine/CameraBase.h"
#include "ai_space.h"
#include "script_engine.h"
#include "level.h"

BOOL g_legs_enabled = FALSE;
float legs_fwd_offset = -0.5f;
BOOL legs_attach_to_camera = TRUE;
BOOL g_body_pivot = TRUE;
BOOL g_body_natural_turn = TRUE;
float g_body_eye_forward = 0.5f;
BOOL g_body_eye_auto = TRUE;
extern int showActorBody;
extern xr_unordered_set<CDemoRecord*> pDemoRecords;

float body_pivot_eye_distance(CActor* actor)
{
    if (!g_body_eye_auto) return g_body_eye_forward;
    // Preserve the original outfit/global body-to-eye spacing, but move the
    // eye ahead of the fixed body root rather than orbiting the mesh behind it.
    PIItem outfit = actor->inventory().ItemFromSlot(OUTFIT_SLOT);
    shared_str section = outfit ? outfit->object().cNameSect() : shared_str("actor");
    const float offset = pSettings->line_exist(section, "legs_fwd_offset")
        ? pSettings->r_float(section, "legs_fwd_offset") : legs_fwd_offset;
    return _valid(offset) ? clampr(-offset, -2.f, 2.f) : 0.5f;
}

bool body_pivot_active(CActor* actor)
{
    return actor && g_body_pivot && g_legs_enabled && showActorBody == 0
        && IsGameTypeSingle() && actor == Level().CurrentControlEntity()
        && actor->active_cam() == eacFirstEye && actor->g_Alive()
        && !actor->Holder() && !(actor->MovingState() & mcClimb)
        && pDemoRecords.empty();
}

void player_legs_controller::destroy()
{
    m_collision_retreat = 0.f;
    m_retreat_frame = u32(-1);
    for (IKinematics** model : { &m_model, &m_shadow_model })
    {
        if (!*model)
            continue;

        if (IRenderVisual* v = (*model)->dcast_RenderVisual())
            ::Render->model_Delete(v);

        *model = nullptr;
    }

    m_visual_name = "";
}

bool player_legs_controller::resolve_config(CActor* actor, shared_str& sect, shared_str& model)
{
    PIItem outfit = actor->inventory().ItemFromSlot(OUTFIT_SLOT);
    shared_str current_outfit = outfit
        ? outfit->object().cNameSect()
        : shared_str("actor");

    if (m_last_outfit_sect == current_outfit)
    {
        sect = m_last_outfit_sect;
        model = m_last_model;
        return true;
    }

    m_last_outfit_sect = current_outfit;

    if (outfit)
    {
        if (pSettings->line_exist(m_last_outfit_sect, "legs_visual"))
        {
            sect = m_last_outfit_sect;
            m_last_model = pSettings->r_string(m_last_outfit_sect, "legs_visual");
            model = m_last_model;
            return true;
        }

        if (pSettings->line_exist(m_last_outfit_sect, "actor_visual"))
        {
            sect = m_last_outfit_sect;
            m_last_model = pSettings->r_string(m_last_outfit_sect, "actor_visual");
            model = m_last_model;
            return true;
        }

        warn_once("outfit [%s] has no legs_visual or actor_visual, fallback to default", m_last_outfit_sect.c_str());
    }

    // default
    if (pSettings->line_exist("actor", "legs_visual"))
    {
        sect = m_last_outfit_sect;
        m_last_model = pSettings->r_string(m_last_outfit_sect, "legs_visual");
        model = m_last_model;
        return true;
    }

    if (pSettings->line_exist(m_last_outfit_sect, "visual"))
    {
        sect = m_last_outfit_sect;
        m_last_model = pSettings->r_string(m_last_outfit_sect, "visual");
        model = m_last_model;
        return true;
    }

    warn_once("actor has no legs_visual or visual");

    return false;
}

void player_legs_controller::warn_once(const char* fmt, ...)
{
    string512 buf;
    va_list args;
    va_start(args, fmt);
    vsprintf_s(buf, sizeof(buf), fmt, args);
    va_end(args);

    xr_string s = make_string("! [player_legs] %s", buf).c_str();
    static xr_unordered_flat_set<xr_string> warnings;
    if (warnings.find(s) == warnings.end())
    {
        warnings.insert(s);
        Msg(s.c_str());
    }
}

bool player_legs_controller::ensure_model(const shared_str& sect, const shared_str& model)
{
    if (m_model && m_visual_name == model)
        return true;

    destroy();

    // Separate instance for the shadow pass: the full body shadow must not touch the hidden bones
    // of the main model, which is drawn again after shadows (emissive, forward passes)
    for (IKinematics** target : { &m_model, &m_shadow_model })
    {
        IRenderVisual* raw = ::Render->model_Create(model.c_str());
        if (!raw)
        {
            destroy();
            warn_once("failed to create model [%s]", model.c_str());
            return false;
        }

        IKinematics* K = smart_cast<IKinematics*>(raw);
        if (!K)
        {
            ::Render->model_Delete(raw);
            destroy();
            warn_once("model [%s] is not a skeleton", model.c_str());
            return false;
        }

        *target = K;
    }

    m_visual_name = model;

    if (pSettings->line_exist(sect, "legs_fwd_offset"))
        m_fwd_offset = pSettings->r_float(sect, "legs_fwd_offset");
    else
        m_fwd_offset = std::nullopt;

    ::luabind::functor<void> funct;
    if (ai().script_engine().functor("_G.player_legs_controller__OnCreateLegs", funct))
        funct(sect.c_str());

    return true;
}

// TODO : clean up later
float legs_spine_offset_y = 0.1f;
void player_legs_controller::copy_bones_from_actor(CActor* actor)
{
    if (!actor || !m_model || !m_shadow_model)
        return;

    IKinematics* actor_K = actor->Visual()->dcast_PKinematics();
    if (!actor_K)
        return;

    actor_K->CalculateBones(TRUE);

    // Full body for shadows
    m_shadow_model->CopyBonesFrom(actor_K);

    // Legs only for the main pass
    m_model->CopyBonesFrom(actor_K);

    if (auto BoneID = m_model->LL_BoneID("bip01_spine"); BoneID != BI_NONE)
    {
        auto& BoneInstance = m_model->LL_GetData(BoneID);
        auto& transform = m_model->LL_GetTransform(BoneInstance.GetParentID());
        transform.c.y += legs_spine_offset_y;
        m_model->Bone_Calculate(&BoneInstance, &transform);
    }

    static LPCSTR bonesToHide[] = { "bip01_neck", "bip01_l_upperarm", "bip01_r_upperarm" };
    for (const auto& bone : bonesToHide)
    {
        u16 bone_id = m_model->LL_BoneID(bone);
        if (bone_id != BI_NONE)
        {
            m_model->LL_SetBoneVisible(bone_id, false, true);
        }
    }
}

void player_legs_controller::record_camera_collision(const Fvector& requested, const Fvector& resolved)
{
    if (m_collision_frame != Device.dwFrame)
    {
        m_camera_collision.set(0.f, 0.f, 0.f);
        m_collision_frame = Device.dwFrame;
    }
    Fvector delta;
    delta.sub(resolved, requested);
    if (_valid(delta)) m_camera_collision.add(delta);
}

void player_legs_controller::update(CActor* actor, bool isShadowPass)
{
    actor->XFORMShadow.set(actor->XFORM());

    if (!g_legs_enabled || showActorBody != 0 || !actor)
    {
        destroy();
        return;
    }

    if (actor->Holder() != nullptr)
    {
        destroy();
        return;
    }

    shared_str sect;
    shared_str model;
    if (!resolve_config(actor, sect, model))
    {
        destroy();
        return;
    }

    if (!ensure_model(sect, model))
        return;

    // Bones of both models are set up once per frame, outside of the shadow pass
    if (!isShadowPass)
        copy_bones_from_actor(actor);

    m_legs_transform.set(actor->XFORM());
    // Keep the normal body pivot. Only actual camera collision may retreat
    // the visual model; camera bob, lean and pitch never drive this offset.
    if (body_pivot_active(actor))
    {
        Fvector forward = actor->XFORM().k;
        forward.y = 0.f;
        forward.normalize_safe();
        if (m_retreat_frame != Device.dwFrame)
        {
            const float required = m_collision_frame == Device.dwFrame
                ? -m_camera_collision.dotproduct(forward) : 0.f;
            if (m_retreat_frame + 1 != Device.dwFrame) m_collision_retreat = 0.f;
            m_collision_retreat = body_pivot::retreat(m_collision_retreat, required, Device.fTimeDelta);
            m_retreat_frame = Device.dwFrame;
        }
        m_legs_transform.c.mad(forward, -m_collision_retreat);
        actor->XFORMShadow.set(m_legs_transform);
        return;
    }
    m_collision_retreat = 0.f;
    m_retreat_frame = u32(-1);
    if (legs_attach_to_camera && pDemoRecords.empty())
        m_legs_transform.c.set(Device.vCameraPosition.x, m_legs_transform.c.y, Device.vCameraPosition.z);

    Fvector fwd = m_legs_transform.k;
    fwd.y = 0.f;
    fwd.normalize_safe();

    float offset = m_fwd_offset.has_value() ? m_fwd_offset.value() : legs_fwd_offset;
    m_legs_transform.c.mad(fwd, offset);

    // Move actor's XFORM for correct shadow placement
    actor->XFORMShadow.translate_over(m_legs_transform.c);
}

void player_legs_controller::render(IDSGraphManager* DM, bool isShadowPass)
{
    IKinematics* model = isShadowPass ? m_shadow_model : m_model;
    if (!model)
        return;

    CActor* actor = Actor();
    if (!actor)
        return;

    u32 move_state = actor->MovingState();
    if (move_state & mcClimb)
        return;

    IRenderVisual* visual = model->dcast_RenderVisual();
    if (!visual)
        return;

    DM->add_Dynamic(visual, &m_legs_transform);
}
