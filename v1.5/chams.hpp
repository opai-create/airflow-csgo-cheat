#pragma once

#define dme_original decltype(&hooks::vmt::draw_model_execute)

struct hook_data_t
{
	dme_original original{};
	void* ecx{};
	void* edx{};
	void* ctx{};
	draw_model_state_t state{};
	model_render_info_t info{};
	matrix3x4_t* bone_to_world{};

	INLINE void init(dme_original original, void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world)
	{
		this->original = original;
		this->ecx = ecx;
		this->edx = edx;
		this->ctx = ctx;
		this->state = state;
		this->info = info;
		this->bone_to_world = bone_to_world;
	}

	INLINE void call_original(matrix3x4_t* matrix = nullptr)
	{
		original(ecx, edx, ctx, state, info, matrix == nullptr ? bone_to_world : matrix);
	}

	INLINE void reset()
	{
		original = nullptr;
		ecx = nullptr;
		edx = nullptr;
		ctx = nullptr;
		state = {};
		info = {};
		bone_to_world = nullptr;
	}
};

struct shot_record_t
{
	float time{};
	float alpha = 1.f;

	model_render_info_t info{};
	draw_model_state_t state{};

	vec3_t origin{};

	alignas(16) matrix3x4_t bones[256]{};
	alignas(16) matrix3x4_t current_bones {};
};

class c_chams
{
private:
	std::string old_base_texture{};

	i_material* tye_dye{};
	i_material* materials[8]{};

	hook_data_t hook_data{};

	std::deque<shot_record_t> shots{};

	bool draw_model(chams_t& chams, matrix3x4_t* matrix = nullptr, float alpha = 1.f, bool xqz = false);
	bool should_draw();

public:

	INLINE void reset()
	{
		old_base_texture = "";
		hook_data.reset();
		shots.clear();
	}
	
	void draw_shot_records();
	void add_shot_record(c_cs_player* player, matrix3x4_t* matrix);
	void init_materials();
	void on_draw_model_execute(dme_original original, void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world);
};

#ifdef _DEBUG
inline auto CHAMS = std::make_unique<c_chams>();
#else
CREATE_DUMMY_PTR(c_chams);
DECLARE_XORED_PTR(c_chams, GET_XOR_KEYUI32);

#define CHAMS XORED_PTR(c_chams)
#endif