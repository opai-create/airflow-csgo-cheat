#pragma once
#include "../sdk/c_material.h"
#include "../sdk/c_studio_model.h"
#include "../tools/utils_macro.h"

enum studio_flags_t
{
	studio_none = 0x00000000,
	studio_should_render = 0x00000001,
	studio_viewxformattachments = 0x00000002,
	studio_drawtranslucentsubmodels = 0x00000004,
	studio_twopass = 0x00000008,
	studio_static_lighting = 0x00000010,
	studio_wireframe = 0x00000020,
	studio_item_blink = 0x00000040,
	studio_noshadows = 0x00000080,
	studio_wireframe_vcollide = 0x00000100,
	studio_nolighting_or_cubemap = 0x00000200,
	studio_skip_flexes = 0x00000400,
	studio_donotmodifystencilstate = 0x00000800,
	studio_skip_decals = 0x10000000,
	studio_shadowtexture = 0x20000000,
	studio_shadowdepthtexture = 0x40000000,
	studio_transparency = 0x80000000,

	render_glow_and_shadow = (studio_should_render | studio_skip_flexes | studio_donotmodifystencilstate | studio_nolighting_or_cubemap | studio_skip_decals),
};

class c_renderable;

struct draw_model_state_t
{
	void* studio_hdr;
	void* studio_hdr_data;
	void* renderable;
	const matrix3x4_t* model_to_world;
	void* decals;
	int draw_flags;
	int lod;

	inline draw_model_state_t operator=(const draw_model_state_t& other)
	{
		std::memcpy(this, &other, sizeof(draw_model_state_t));
		return *this;
	}
};

struct renderable_info_t
{
	c_renderable* m_pRenderable;
	void* m_pAlphaProperty;
	int m_EnumCount; // Have interfaces been added to a particular shadow yet?
	int m_nRenderFrame;
	unsigned short m_FirstShadow;                // The first shadow caster that cast on it
	unsigned short m_LeafList;                   // What leafs is it in?
	short m_Area;                                // -1 if the renderable spans multiple areas.
	uint16_t m_Flags;                            // rendering flags
	uint16_t m_bRenderInFastReflection : 1;      // Should we render in the "fast" reflection?
	uint16_t m_bDisableShadowDepthRendering : 1; // Should we not render into the shadow depth map?
	uint16_t m_bDisableCSMRendering : 1;         // Should we not render into the CSM?
	uint16_t m_bDisableShadowDepthCaching : 1;   // Should we not be cached in the shadow depth map?
	uint16_t m_nSplitscreenEnabled : 2;          // splitscreen rendering flags
	uint16_t m_nTranslucencyType : 2;            // RenderableTranslucencyType_t
	uint16_t m_nModelType : 8;                   // RenderableModelType_t
	uint16_t m_vecBloatedAbsMins;                // Use this for tree insertion
	uint16_t m_vecBloatedAbsMaxs;
	uint16_t m_vecAbsMins; // NOTE: These members are not threadsafe!!
	uint16_t m_vecAbsMaxs; // They can be updated from any viewpoint (based on RENDER_FLAGS_BOUNDS_VALID)
};

struct model_render_info_t
{
	vector3d origin;
	vector3d angles;

	padding(4);

	void* renderable;
	const model_t* model;
	const matrix3x4_t* model_to_world;
	const matrix3x4_t* lightning_offset;
	const vector3d* lightning_origin;
	int flags;
	int entity_index;
	int skin;
	int body;
	int hitboxset;
	unsigned short instance;

	inline model_render_info_t operator=(const model_render_info_t& other)
	{
		memcpy(this, &other, sizeof(model_render_info_t));
		return *this;
	}
};

class c_model_render
{
public:
	virtual int unk() = 0;
	virtual void forced_material_override(c_material* material, int type = 0, int overrides = 0) = 0;

	void draw_model_execute(void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world = NULL)
	{
		using fn = void(__thiscall*)(void*, void*, const draw_model_state_t&, const model_render_info_t&, matrix3x4_t*);
		g_memory->getvfunc< fn >(this, 21)(this, ctx, state, info, bone_to_world);
	}

	void suppress_engine_lighting(bool suppress)
	{
		using fn = void(__thiscall*)(void*, bool);
		g_memory->getvfunc< fn >(this, 24)(this, suppress);
	}
};