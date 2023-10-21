#pragma once
#include <array>

#include "../../base/sdk/entity.h"

#include "animfix.h"
#include "rage_tools.h"
#include "resolver.h"

class c_csplayer;

struct aim_shot_record_t
{
	bool fire{};
	bool impact_fire{};

	int safety{};
	int damage{};
	int hitgroup{};
	int hitbox{};

	float time{};
	float init_time{};
	float hitchance{};

	int index{};

	vector3d start{};
	vector3d impact{};
	vector3d point{};

	records_t record{};
	resolver_info_t resolver{};
};

struct point_t
{
	bool filled{};
	bool body{};
	bool limbs{};
	bool center{};
	bool lethal{};
	bool predictive{};
	bool good_spread{};

	int hitbox = -1;
	int damage{};
	int safety{};
	int chance_ticks = 0;
	int chance_ticks2 = 0;

	int priority = 0;

	float accuracy{};
	float hitchance{};
	float old_hitchance{};

	records_t* record{};
	vector3d position{};

	__forceinline point_t()
	{
	}

	__forceinline point_t(int hitbox, bool center, int damage, records_t* record, const vector3d& point_position)
	{
		this->filled = true;
		this->center = center;
		this->body = hitbox >= hitbox_pelvis && hitbox <= hitbox_chest;
		this->limbs = hitbox >= hitbox_left_thigh && hitbox <= hitbox_right_forearm;
		this->hitbox = hitbox;
		this->damage = damage;
		this->record = record;
		this->position = point_position;
	}

	__forceinline void reset()
	{
		filled = false;
		body = false;
		limbs = false;
		center = false;
		lethal = false;
		predictive = false;
		good_spread = false;

		hitbox = -1;
		damage = 0;
		safety = 0;
		priority = 0;
		chance_ticks = 0;
		chance_ticks2 = 0;

		accuracy = 0.f;
		hitchance = 0.f;
		old_hitchance = 0.f;

		record = nullptr;
		position.reset();
	}
};

struct restore_record_t
{
	float duck{};
	float lby{};
	float sim_time{};
	float collision_change_time{};

	vector3d angles{};
	vector3d origin{};
	vector3d absorigin{};
	vector3d collision_change_origin{};
	vector3d velocity{};

	vector3d bbmin{};
	vector3d bbmax{};

	uint32_t ground_entity{};

	matrix3x4_t bonecache[128]{};

	std::array< float, 24 > poses{};

	bool filled{ false };

	__forceinline void reset()
	{
		if (!filled)
			return;

		duck = 0.f;
		lby = 0.f;
		sim_time = 0.f;
		collision_change_time = 0.f;

		ground_entity = 0;

		angles.reset();
		origin.reset();
		absorigin.reset();
		velocity.reset();

		bbmin.reset();
		bbmax.reset();
		collision_change_origin.reset();

		for (auto& i : bonecache)
		{
			for (int j = 0; j < 3; ++j)
			{
				for (int x = 0; x < 4; ++x)
					i.mat[j][x] = 0.f;
			}
		}

		poses = {};

		filled = false;
	}
};

struct aim_cache_t
{
	c_csplayer* player{};
	std::vector< point_t > points{};
	point_t best_point{};

	inline void reset()
	{
		this->player = nullptr;
		this->points.clear();
		this->best_point.reset();
	}
};

struct knife_point_t
{
	bool stab{};
	records_t* record{};
	vector3d angle{};
};

class c_rage_bot
{
private:
	bool reset_data{};
	bool reset_scan_data{};

	int last_shot_cmd{};

	std::vector< aim_shot_record_t > shots{};

	struct table_t
	{
		uint8_t swing[2][2][2];
		uint8_t stab[2][2];
	};

	const table_t knife_dmg{ { { { 25, 90 }, { 21, 76 } }, { { 40, 90 }, { 34, 76 } } }, { { 65, 180 }, { 55, 153 } } };

	std::array<vector3d, 12> knife_ang{
		vector3d{ 0.f, 0.f, 0.f },
		vector3d{ 0.f, -90.f, 0.f },
		vector3d{ 0.f, 90.f, 0.f },
		vector3d{ 0.f, 180.f, 0.f },
		vector3d{ -80.f, 0.f, 0.f },
		vector3d{ -80.f, -90.f, 0.f },
		vector3d{ -80.f, 90.f, 0.f },
		vector3d{ -80.f, 180.f, 0.f },
		vector3d{ 80.f, 0.f, 0.f },
		vector3d{ 80.f, -90.f, 0.f },
		vector3d{ 80.f, 90.f, 0.f },
		vector3d{ 80.f, 180.f, 0.f }
	};

	aim_cache_t aim_cache[65]{};

	void add_shot_record(c_csplayer* player, const point_t& best);
	bool knife_is_behind(records_t* record);
	bool knife_trace(vector3d dir, bool stab, c_game_trace* trace);
	bool can_knife(records_t* record, vector3d angle, bool& stab);
	void knife_bot();
	void zeus_bot();

	__forceinline void clear_player_cache()
	{
		for (auto& cache : aim_cache)
		{
			if (!cache.points.empty())
				cache.points.clear();

			if (cache.best_point.filled)
				cache.best_point.reset();

			if (cache.player)
				cache.player = nullptr;
		}
	}

	void weapon_fire(c_game_event* event);
	void bullet_impact(c_game_event* event);
	void player_hurt(c_game_event* event);
	void round_start(c_game_event* event);

	void predict_eye_pos();
public:
	int missed_shots[65]{};
	restore_record_t backup[65]{};
	restore_record_t backup_global[65]{};

	bool working{};
	bool firing{};
	bool should_slide{};
	bool force_accuracy{};
	bool stopping{};
	bool pred_stopping{};
	bool debug_aimbot = false;
	c_csplayer* target{};

	int predictive_stop_ticks{};
	vector3d predicted_eye_pos{};

	rage_weapon_t weapon_config{};

	bool should_stop(bool shoot_check = true);
	bool auto_stop();
	void start_stop();

	std::vector< int > get_hitboxes();

	void store_global(c_csplayer* player);
	void restore_global(c_csplayer* player);
	void store(c_csplayer* player);
	void set_record(c_csplayer* player, records_t* record, matrix3x4_t* matrix = nullptr);
	void restore(c_csplayer* player);

	void on_cm_start();
	void on_cm_end();

	void on_game_events(c_game_event* event);
	void proceed_aimbot();
	void on_pre_predict();
	void on_predict_start();
	void on_local_death();
	void on_changed_map();

	int get_safety_count(c_csplayer* player, point_t& point);
	int get_min_damage(c_csplayer* player);

	__forceinline bool can_dt()
	{
		return g_cfg.binds[dt_b].toggled && g_ctx.weapon->item_definition_index() != weapon_awp && g_ctx.weapon->item_definition_index() != weapon_ssg08;
	}
};