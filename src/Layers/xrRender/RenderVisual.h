#ifndef	RenderVisual_included
#define	RenderVisual_included
#pragma once

#include "_flags.h"
#include "_stl_extensions.h"

class IKinematics;
class IKinematicsAnimated;
class IParticleCustom;
struct vis_data;

enum IRenderVisualFlags
{
	eIgnoreOptimization = (1 << 0),
	eNoShadow = (1 << 1),
};

class IRenderVisual
{
public:
	IRenderVisual() { flags.zero(); }

	virtual ~IRenderVisual()
	{
	}

	virtual vis_data& getVisData() = 0;
	virtual u32 getType() = 0;

	Flags16 flags;

#ifdef DEBUG
	virtual shared_str getDebugName() = 0;
#endif
	virtual LPCSTR getDebugShader() { return nullptr; }
	virtual LPCSTR getDebugTexture() { return nullptr; }
	
	virtual LPCSTR getDebugShaderDef() { return nullptr; }
	virtual LPCSTR getDebugTextureDef() { return nullptr; }

	virtual xr_vector<IRenderVisual*>* get_children() { return nullptr; };

	virtual void SetShaderTexture(LPCSTR shader, LPCSTR texture) {};
	virtual void ResetShaderTexture() {};
	virtual void MarkAsHot(bool is_hot) {};				//--DSR-- HeatVision
	virtual void MarkAsGlowing(bool is_glowing) {};		//--DSR-- SilencerOverheat

	virtual IRenderVisual* dcast_RenderVisual() { return this; }
	virtual IKinematics* dcast_PKinematics() { return 0; }
	virtual IKinematicsAnimated* dcast_PKinematicsAnimated() { return 0; }
	virtual IParticleCustom* dcast_ParticleCustom() { return 0; }
};

#endif	//	RenderVisual_included
