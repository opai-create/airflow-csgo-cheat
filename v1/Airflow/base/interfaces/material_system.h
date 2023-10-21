#pragma once
#include "../tools/math.h"

#include "../sdk/studio_hdr.h"

enum studiorender_flags_t
{
	studiorender_draw_entire_model = 0,
	studiorender_draw_opaque_only = 0x01,
	studiorender_draw_translucent_only = 0x02,
	studiorender_draw_group_mask = 0x03,
	studiorender_draw_no_flexes = 0x04,
	studiorender_draw_static_lighting = 0x08,
	studiorender_draw_accuratetime = 0x10,
	studiorender_draw_no_shadows = 0x20,
	studiorender_draw_get_perf_stats = 0x40,
	studiorender_draw_wireframe = 0x80,
	studiorender_draw_item_blink = 0x100,
	studiorender_shadowdepthtexture = 0x200,
	studiorender_unused = 0x400,
	studiorender_skip_decals = 0x800,
	studiorender_model_is_cacheable = 0x1000,
	studiorender_shadowdepthtexture_include_translucent_materials = 0x2000,
	studiorender_no_primary_draw = 0x4000,
	studiorender_ssaodepthtexture = 0x8000,
};

enum shader_stencil_op_t
{
	shader_stencilop_keep = 1,
	shader_stencilop_zero = 2,
	shader_stencilop_set_to_reference = 3,
	shader_stencilop_increment_clamp = 4,
	shader_stencilop_decrement_clamp = 5,
	shader_stencilop_invert = 6,
	shader_stencilop_increment_wrap = 7,
	shader_stencilop_decrement_wrap = 8,
	shader_stencilop_force_dword = 0x7FFFFFFF
};

enum shader_stencil_func_t
{
	shader_stencilfunc_never = 1,
	shader_stencilfunc_less = 2,
	shader_stencilfunc_equal = 3,
	shader_stencilfunc_lequal = 4,
	shader_stencilfunc_greater = 5,
	shader_stencilfunc_notequal = 6,
	shader_stencilfunc_gequal = 7,
	shader_stencilfunc_always = 8,
	shader_stencilfunc_force_dword = 0x7FFFFFFF
};

enum renderable_lightning_model_t
{
	lighting_model_none = -1,
	lighting_model_standard = 0,
	lighting_model_static_prop,
	lighting_model_physics_prop,

	lighting_model_count,
};

struct light_desc_t
{
	int type;
	vector3d color;
	vector3d position;
	vector3d direction;
	float range;
	float falloff;
	float attenuation0;
	float attenuation1;
	float attenuation2;
	float theta;
	float phi;
	float theta_dot;
	float phi_dot;
	float over_theta_dot_minus_dot;
	std::uint32_t flags;

protected:
	float range_squared;
};

struct render_instance_t
{
	uint8_t alpha;
};

struct flash_light_instance_t
{
	c_material* debug_material;
	char padding[248];
	matrix3x4_t world_to_texture;
	void* flash_depth_light_texture;
};

struct studio_array_data_t
{
	studio_hdr_t* studio_hdr;
	studio_hw_data_t* hardware_data;
	void* instance_data;
	int count;
};

struct studio_model_array_info2_t
{
	int flash_count;
	flash_light_instance_t* flashlights;
};

struct studio_model_array_info_t : public studio_model_array_info2_t
{
	studio_hdr_t* studio_hdr;
	studio_hw_data_t* hardware_data;
};

struct model_render_system_data_t
{
	void* renderable;
	void* renderable_model;
	render_instance_t instance_data;
};

struct shader_stencil_state_t
{
	bool enable;
	shader_stencil_op_t fail_op;
	shader_stencil_op_t z_fail_op;
	shader_stencil_op_t pass_op;
	shader_stencil_func_t compare_func;
	int reference_value;
	uint32_t test_mask;
	uint32_t write_masl;

	shader_stencil_state_t()
	{
		enable = false;
		fail_op = z_fail_op = pass_op = shader_stencilop_keep;
		compare_func = shader_stencilfunc_always;
		reference_value = 0;
		test_mask = write_masl = 0xFFFFFFFF;
	}
};

struct studio_shadow_array_instance_data_t
{
	int lod;
	int body;
	int skin;
	matrix3x4_t* pose_to_world;
	float* flex_weights;
	float* delayed_flex_weights;
};

struct color_mesh_info_t
{
	void* mesh;
	void* all_allocator;
	int vert_offset;
	int num_verts;
};

struct material_lightning_state_t
{
	vector3d aimbient_cube[6];
	vector3d lightning_origin;
	int light_count;
	light_desc_t light_desc[4];
};

struct studio_array_instance_data_t : public studio_shadow_array_instance_data_t
{
	material_lightning_state_t* light_state;
	material_lightning_state_t* decal_light_state;
	void* env_cubemap_textuer;
	void* decals;
	uint32_t flash_usage;
	shader_stencil_state_t* stencil_state;
	color_mesh_info_t* color_mesh_info;
	bool mesh_has_light_only;
	vector4d diffuse_modulation;
};

struct model_list_mode_t
{
	model_render_system_data_t entry;
	int32_t initial_list_index : 24;
	uint32_t bone_merge : 1;
	int32_t lod : 7;
	shader_stencil_state_t* stencil_state;
	model_list_mode_t* next;
};

struct render_model_info_t : public studio_array_instance_data_t
{
	model_render_system_data_t entry;
	unsigned short instance;
	matrix3x4_t* bone_to_world;
	uint32_t list_idx : 24;
	uint32_t setup_bones_only : 1;
	uint32_t bone_merge : 1;
};

struct brush_array_instance_data_t
{
	matrix3x4a_t* brush_to_world;
	const model_t* brush_model;
	vector4d diffuse_modulation;
	shader_stencil_state_t* stencil_state;
};

struct model_list_by_type_t : public studio_model_array_info_t
{
	renderable_lightning_model_t light_model;
	const model_t* model;
	model_list_mode_t* first_node;
	int count;
	int setup_bone_count;
	uint32_t parent_depth : 31;
	uint32_t wants_stencil : 1;
	render_model_info_t* render_models;
	model_list_by_type_t* next_lightning_model;

	model_list_by_type_t& operator=(const model_list_by_type_t& rhs)
	{
		memcpy(this, &rhs, sizeof(model_list_by_type_t));
		return *this;
	}

	model_list_by_type_t()
	{
	}

	model_list_by_type_t(const model_list_by_type_t& rhs)
	{
		std::memcpy(this, &rhs, sizeof(model_list_by_type_t));
	}
};

class IShaderInit
{
public:
	// Loads up a texture
	virtual void LoadTexture(c_material_var* pTextureVar, const char* pTextureGroupName, int nAdditionalCreationFlags = 0) = 0;
	virtual void LoadBumpMap(c_material_var* pTextureVar, const char* pTextureGroupName, int nAdditionalCreationFlags = 0) = 0;
	virtual void LoadCubeMap(c_material_var** ppParams, c_material_var* pTextureVar, int nAdditionalCreationFlags = 0) = 0;
};

enum ShaderParamType_t
{
	SHADER_PARAM_TYPE_TEXTURE,
	SHADER_PARAM_TYPE_INTEGER,
	SHADER_PARAM_TYPE_COLOR,
	SHADER_PARAM_TYPE_VEC2,
	SHADER_PARAM_TYPE_VEC3,
	SHADER_PARAM_TYPE_VEC4,
	SHADER_PARAM_TYPE_ENVMAP, // obsolete
	SHADER_PARAM_TYPE_FLOAT,
	SHADER_PARAM_TYPE_BOOL,
	SHADER_PARAM_TYPE_FOURCC,
	SHADER_PARAM_TYPE_MATRIX,
	SHADER_PARAM_TYPE_MATERIAL,
	SHADER_PARAM_TYPE_STRING,
};

enum ShaderDepthFunc_t
{
	SHADER_DEPTHFUNC_NEVER,
	SHADER_DEPTHFUNC_NEARER,
	SHADER_DEPTHFUNC_EQUAL,
	SHADER_DEPTHFUNC_NEAREROREQUAL,
	SHADER_DEPTHFUNC_FARTHER,
	SHADER_DEPTHFUNC_NOTEQUAL,
	SHADER_DEPTHFUNC_FARTHEROREQUAL,
	SHADER_DEPTHFUNC_ALWAYS
};

enum ShaderBlendFactor_t
{
	SHADER_BLEND_ZERO,
	SHADER_BLEND_ONE,
	SHADER_BLEND_DST_COLOR,
	SHADER_BLEND_ONE_MINUS_DST_COLOR,
	SHADER_BLEND_SRC_ALPHA,
	SHADER_BLEND_ONE_MINUS_SRC_ALPHA,
	SHADER_BLEND_DST_ALPHA,
	SHADER_BLEND_ONE_MINUS_DST_ALPHA,
	SHADER_BLEND_SRC_ALPHA_SATURATE,
	SHADER_BLEND_SRC_COLOR,
	SHADER_BLEND_ONE_MINUS_SRC_COLOR
};

enum ShaderBlendOp_t
{
	SHADER_BLEND_OP_ADD,
	SHADER_BLEND_OP_SUBTRACT,
	SHADER_BLEND_OP_REVSUBTRACT,
	SHADER_BLEND_OP_MIN,
	SHADER_BLEND_OP_MAX
};

enum ShaderAlphaFunc_t
{
	SHADER_ALPHAFUNC_NEVER,
	SHADER_ALPHAFUNC_LESS,
	SHADER_ALPHAFUNC_EQUAL,
	SHADER_ALPHAFUNC_LEQUAL,
	SHADER_ALPHAFUNC_GREATER,
	SHADER_ALPHAFUNC_NOTEQUAL,
	SHADER_ALPHAFUNC_GEQUAL,
	SHADER_ALPHAFUNC_ALWAYS
};

enum ShaderTexChannel_t
{
	SHADER_TEXCHANNEL_COLOR = 0,
	SHADER_TEXCHANNEL_ALPHA
};

enum ShaderPolyModeFace_t
{
	SHADER_POLYMODEFACE_FRONT,
	SHADER_POLYMODEFACE_BACK,
	SHADER_POLYMODEFACE_FRONT_AND_BACK,
};

enum ShaderPolyMode_t
{
	SHADER_POLYMODE_POINT,
	SHADER_POLYMODE_LINE,
	SHADER_POLYMODE_FILL
};

enum ShaderFogMode_t
{
	SHADER_FOGMODE_DISABLED = 0,
	SHADER_FOGMODE_OO_OVERBRIGHT,
	SHADER_FOGMODE_BLACK,
	SHADER_FOGMODE_GREY,
	SHADER_FOGMODE_FOGCOLOR,
	SHADER_FOGMODE_WHITE,
	SHADER_FOGMODE_NUMFOGMODES
};

// m_ZBias has only two bits in ShadowState_t, so be careful extending this enum
enum PolygonOffsetMode_t
{
	SHADER_POLYOFFSET_DISABLE = 0x0,
	SHADER_POLYOFFSET_DECAL = 0x1,
	SHADER_POLYOFFSET_SHADOW_BIAS = 0x2,
	SHADER_POLYOFFSET_RESERVED = 0x3 // Reserved for future use
};

enum Sampler_t
{
	SHADER_SAMPLER_INVALID = -1,

	SHADER_SAMPLER0 = 0,
	SHADER_SAMPLER1,
	SHADER_SAMPLER2,
	SHADER_SAMPLER3,
	SHADER_SAMPLER4,
	SHADER_SAMPLER5,
	SHADER_SAMPLER6,
	SHADER_SAMPLER7,
	SHADER_SAMPLER8,
	SHADER_SAMPLER9,
	SHADER_SAMPLER10,
	SHADER_SAMPLER11,
	SHADER_SAMPLER12,
	SHADER_SAMPLER13,
	SHADER_SAMPLER14,
	SHADER_SAMPLER15,

	SHADER_SAMPLER_COUNT,
};

enum VertexTextureSampler_t
{
	SHADER_VERTEXTEXTURE_SAMPLER0 = 0,
	SHADER_VERTEXTEXTURE_SAMPLER1,
	SHADER_VERTEXTEXTURE_SAMPLER2,
	SHADER_VERTEXTEXTURE_SAMPLER3,
};

enum VertexCompressionType_t
{
	// This indicates an uninitialized VertexCompressionType_t value
	VERTEX_COMPRESSION_INVALID = 0xFFFFFFFF,

	// 'VERTEX_COMPRESSION_NONE' means that no elements of a vertex are compressed
	VERTEX_COMPRESSION_NONE = 0,

	// Currently (more stuff may be added as needed), 'VERTEX_COMPRESSION_FULL' means:
	//  - if a vertex contains VERTEX_ELEMENT_NORMAL, this is compressed
	//    (see CVertexBuilder::CompressedNormal3f)
	//  - if a vertex contains VERTEX_ELEMENT_USERDATA4 (and a normal - together defining a tangent
	//    frame, with the binormal reconstructed in the vertex shader), this is compressed
	//    (see CVertexBuilder::CompressedUserData)
	//  - if a vertex contains VERTEX_ELEMENT_BONEWEIGHTSx, this is compressed
	//    (see CVertexBuilder::CompressedBoneWeight3fv)
	//  - if a vertex contains VERTEX_ELEMENT_TEXCOORD2D_0, this is compressed
	//    (see CVertexBuilder::CompressedTexCoord2fv)
	VERTEX_COMPRESSION_FULL = (1 << 0),
	VERTEX_COMPRESSION_ON = VERTEX_COMPRESSION_FULL,
	// VERTEX_COMPRESSION_NOUV is the same as VERTEX_COMPRESSION_FULL, but does not compress
	// texture coordinates. Some assets use very large texture coordinates, so these cannot be
	// compressed - but the rest of the vertex data can be.
	VERTEX_COMPRESSION_NOUV = (1 << 1),

	VERTEX_COMPRESSION_MASK = (VERTEX_COMPRESSION_FULL | VERTEX_COMPRESSION_NOUV),
};

//-----------------------------------------------------------------------------
// the shader API interface (methods called from shaders)
//-----------------------------------------------------------------------------
class IShaderShadow
{
public:
	// Sets the default *shadow* state
	virtual void SetDefaultState() = 0;

	// Methods related to depth buffering
	virtual void DepthFunc(ShaderDepthFunc_t depthFunc) = 0;
	virtual void EnableDepthWrites(bool bEnable) = 0;
	virtual void EnableDepthTest(bool bEnable) = 0;
	virtual void EnablePolyOffset(PolygonOffsetMode_t nOffsetMode) = 0;

	// Suppresses/activates color writing
	virtual void EnableColorWrites(bool bEnable) = 0;
	virtual void EnableAlphaWrites(bool bEnable) = 0;

	// Methods related to alpha blending
	virtual void EnableBlending(bool bEnable) = 0;
	virtual void EnableBlendingForceOpaque(bool bEnable) = 0; // enables alpha blending on a batch but does not force it to render with translucents
	virtual void BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor) = 0;
	virtual void EnableBlendingSeparateAlpha(bool bEnable) = 0;
	virtual void BlendFuncSeparateAlpha(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor) = 0;
	// More below...

	// Alpha testing
	virtual void EnableAlphaTest(bool bEnable) = 0;
	virtual void AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef /* [0-1] */) = 0;

	// Wireframe/filled polygons
	virtual void PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode) = 0;

	// Back face culling
	virtual void EnableCulling(bool bEnable) = 0;

	// Indicates the vertex format for use with a vertex shader
	// The flags to pass in here come from the VertexFormatFlags_t enum
	// If pTexCoordDimensions is *not* specified, we assume all coordinates
	// are 2-dimensional
	virtual void VertexShaderVertexFormat(unsigned int nFlags, int nTexCoordCount, int* pTexCoordDimensions, int nUserDataSize) = 0;

	// Pixel and vertex shader methods
	virtual void SetVertexShader(const char* pFileName, int nStaticVshIndex) = 0;
	virtual void SetPixelShader(const char* pFileName, int nStaticPshIndex = 0) = 0;

	// Convert from linear to gamma color space on writes to frame buffer.
	virtual void EnableSRGBWrite(bool bEnable) = 0;

	// Convert from gamma to linear on texture fetch.
	virtual void EnableSRGBRead(Sampler_t sampler, bool bEnable) = 0;

	// Per texture unit stuff
	virtual void EnableTexture(Sampler_t sampler, bool bEnable) = 0;

	virtual void FogMode(ShaderFogMode_t fogMode, bool bVertexFog) = 0;

	virtual void DisableFogGammaCorrection(bool bDisable) = 0; // some blending modes won't work properly with corrected fog

	// Alpha to coverage
	virtual void EnableAlphaToCoverage(bool bEnable) = 0;

	// Shadow map filtering
	// virtual void SetShadowDepthFiltering( Sampler_t stage ) = 0;

	// Per vertex texture unit stuff
	virtual void EnableVertexTexture(VertexTextureSampler_t sampler, bool bEnable) = 0;

	// More alpha blending state
	virtual void BlendOp(ShaderBlendOp_t blendOp) = 0;
	virtual void BlendOpSeparateAlpha(ShaderBlendOp_t blendOp) = 0;

	virtual float GetLightMapScaleFactor(void) const = 0;
};

struct ShaderParamInfo_t
{
	const char* m_pName;
	const char* m_pHelp;
	ShaderParamType_t m_Type;
	const char* m_pDefaultValue;
	int m_nFlags;
};

class IShaderDynamicAPI;
class CBasePerMaterialContextData;
class CBasePerInstanceContextData;

class IShader
{
public:
	// Returns the shader name
	virtual char const* GetName() const = 0;

	// returns the shader fallbacks
	virtual char const* GetFallbackShader(c_material_var** params) const = 0;

	// Shader parameters
	virtual int GetParamCount() const = 0;
	virtual const ShaderParamInfo_t& GetParamInfo(int paramIndex) const = 0;

	// These functions must be implemented by the shader
	virtual void InitShaderParams(c_material_var** ppParams, const char* pMaterialName) = 0;
	virtual void InitShaderInstance(c_material_var** ppParams, IShaderInit* pShaderInit, const char* pMaterialName, const char* pTextureGroupName) = 0;
	virtual void DrawElements(c_material_var** params, int nModulationFlags, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData** pContextDataPtr,
		CBasePerInstanceContextData** pInstanceDataPtr) = 0;

	virtual void ExecuteFastPath(int* vsDynIndex, int* psDynIndex, c_material_var** params, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData** pContextDataPtr, BOOL bCSMEnabled)
	{
		*vsDynIndex = -1;
		*psDynIndex = -1;
	}

	// FIXME: Figure out a better way to do this?
	virtual int ComputeModulationFlags(c_material_var** params, IShaderDynamicAPI* pShaderAPI) = 0;
	virtual bool NeedsPowerOfTwoFrameBufferTexture(c_material_var** params, bool bCheckSpecificToThisFrame = true) const = 0;
	virtual bool NeedsFullFrameBufferTexture(c_material_var** params, bool bCheckSpecificToThisFrame) const = 0;
	virtual bool IsTranslucent(c_material_var** params) const = 0;

	virtual int GetFlags() const = 0;

	virtual void SetPPParams(c_material_var** params) = 0;
	virtual void SetModulationFlags(int modulationFlags) = 0;
};

class c_material_system
{
public:
	c_material* create_material(const char* name, c_key_values* key)
	{
		using fn = c_material * (__thiscall*)(void*, const char*, c_key_values*);
		return g_memory->getvfunc< fn >(this, 83)(this, name, key);
	}

	c_material* find_material(const char* material_name, const char* group_name, bool complain = true, const char* complain_prefix = NULL)
	{
		using fn = c_material * (__thiscall*)(void*, const char*, const char*, bool, const char*);
		return g_memory->getvfunc< fn >(this, 84)(this, material_name, group_name, complain, complain_prefix);
	}

	unsigned short first_material()
	{
		using fn = unsigned short(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 86)(this);
	}

	unsigned short next_material(unsigned short h)
	{
		using fn = unsigned short(__thiscall*)(void*, unsigned short h);
		return g_memory->getvfunc< fn >(this, 87)(this, h);
	}

	unsigned short invalid_material()
	{
		using fn = unsigned short(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 88)(this);
	}

	c_material* get_material(unsigned short h)
	{
		using fn = c_material * (__thiscall*)(void*, unsigned short);
		return g_memory->getvfunc< fn >(this, 89)(this, h);
	}

	void* get_render_context()
	{
		using fn = void* (__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 115)(this);
	}
};

class c_key_values_system
{
public:
	virtual void unk() = 0;
	virtual void RegisterSizeofKeyValues(int iSize) = 0;
	virtual void* AllocKeyValuesMemory(int iSize) = 0;
	virtual void FreeKeyValuesMemory(void* pMemory) = 0;
	virtual int GetSymbolForString(const char* szName, bool bCreate = true) = 0;
	virtual const char* GetStringForSymbol(int hSymbol) = 0;
	virtual void AddKeyValuesToMemoryLeakList(void* pMemory, int hSymbolName) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList(void* pMemory) = 0;
	virtual void SetKeyValuesExpressionSymbol(const char* szName, bool bValue) = 0;
	virtual bool GetKeyValuesExpressionSymbol(const char* szName) = 0;
	virtual int GetSymbolForStringCaseSensitive(int& hCaseInsensitiveSymbol, const char* szName, bool bCreate = true) = 0;
};

using key_values_system_fn = c_key_values_system * (__cdecl*)();