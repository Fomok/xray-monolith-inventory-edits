#pragma once

#include "_vector3d.h"
#include "_matrix.h"
#include "CameraBase.h"
#include "IPhysicsShellHolder.h"

class CPhysicsShell;
class CCameraBase;
class IPhysicsShellHolder;

extern XRPHYSICS_API CPhysicsShell* actor_camera_shell;
#ifdef DEBUG
extern XRPHYSICS_API BOOL dbg_draw_camera_collision;
extern XRPHYSICS_API float	camera_collision_character_skin_depth ;
extern XRPHYSICS_API float	camera_collision_character_shift_z ;
#endif
XRPHYSICS_API bool test_camera_box(const Fvector& box_size, const Fmatrix& xform, IPhysicsShellHolder* l_actor);
XRPHYSICS_API void collide_camera(CCameraBase& camera, float _viewport_near, IPhysicsShellHolder* l_actor);
