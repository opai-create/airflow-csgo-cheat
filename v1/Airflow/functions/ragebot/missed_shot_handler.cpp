#include "../features.h"
#include "../../additionals/tinyformat.h"

#include "ragebot.h"

void c_rage_bot::add_shot_record(c_csplayer* player, const point_t& best)
{
	static auto resolver_str = xor_str(", ");
	static auto log_str = xor_str("AIM to %s, %s, %d%%, %dhp, %dt, %dsp%s");

	auto& new_shot = shots.emplace_back();
	new_shot.time = math::ticks_to_time(g_ctx.tick_base);
	new_shot.init_time = 0.f;
	new_shot.impact_fire = false;
	new_shot.fire = false;
	new_shot.damage = -1;
	new_shot.safety = best.safety;
	new_shot.start = g_ctx.eye_position;
	new_shot.hitgroup = -1;
	new_shot.hitchance = best.hitchance;
	new_shot.hitbox = best.hitbox;
	new_shot.record = *best.record;
	new_shot.index = player->index();
	new_shot.resolver = resolver_info[new_shot.index];
	new_shot.point = best.position;

	if (g_cfg.visuals.eventlog.logs & 4)
	{
		const auto& name = player->get_name();

		int diff = math::time_to_ticks(std::abs(best.record->sim_time - player->simulation_time()));

		std::string log = tfm::format(
			log_str.c_str(),
			name,
			rage_tools::hitbox_to_string(best.hitbox),
			(int)(best.hitchance * 100.f),
			best.damage,
			diff,
			best.safety,
#if _DEBUG || ALPHA || BETA
			new_shot.resolver.resolved ? resolver_str + new_shot.resolver.mode : "" ""
#else
			""
#endif
		);

		g_event_logger->add_message(log, -1, true);
	}
}

void c_rage_bot::weapon_fire(c_game_event* event)
{
	if (!g_ctx.local || !g_ctx.local->is_alive())
		return;

	if (this->shots.empty())
		return;

	auto& shot = this->shots.front();
	if (!shot.fire)
		shot.fire = true;
}

void c_rage_bot::bullet_impact(c_game_event* event)
{
	if (!g_ctx.local || !g_ctx.local->is_alive())
		return;

	if (this->shots.empty())
		return;

	auto& shot = this->shots.front();

	if (interfaces::engine->get_player_for_user_id(event->get_int(xor_c("userid"))) != interfaces::engine->get_local_player())
		return;

	const vector3d vec_impact{ event->get_float(xor_c("x")), event->get_float(xor_c("y")), event->get_float(xor_c("z")) };

	bool check = false;
	if (shot.impact_fire)
	{
		if (shot.start.dist_to(vec_impact) > shot.start.dist_to(shot.impact))
			check = true;
	}
	else
		check = true;

	if (!check)
		return;

	shot.impact_fire = true;
	shot.init_time = math::ticks_to_time(g_ctx.tick_base - g_exploits->tickbase_offset());
	shot.impact = vec_impact;
}

void c_rage_bot::player_hurt(c_game_event* event)
{
	if (!g_ctx.local || !g_ctx.local->is_alive())
		return;

	if (interfaces::engine->get_player_for_user_id(event->get_int(xor_c("attacker"))) != interfaces::engine->get_local_player())
		return;

	int user_id = interfaces::engine->get_player_for_user_id(event->get_int(xor_c("userid")));

	c_csplayer* player = (c_csplayer*)interfaces::entity_list->get_entity(user_id);
	if (!player)
		return;

	int group = event->get_int(xor_c("hitgroup"));
	int dmg_health = event->get_int(xor_c("dmg_health"));
	int health = event->get_int(xor_c("health"));

	if (!shots.empty())
	{
		auto& shot = this->shots.front();
		this->shots.erase(this->shots.begin());
	}

	if (g_cfg.visuals.eventlog.logs & 1)
	{
		std::string message{};

		if (group != hitgroup_generic && group != hitgroup_gear)
		{
			message += xor_c("in ");
			message += player->get_name();
			message += xor_c("'s ");
			message += rage_tools::hitgroup_to_string(group);
		}
		else
			message += player->get_name();

		message += xor_c(" for ");
		message += std::to_string(dmg_health);
		message += xor_c("HP");

		g_event_logger->add_message(message, event_hit);
	}
}

void c_rage_bot::round_start(c_game_event* event)
{
	for (auto& m : this->missed_shots)
		m = 0;

	for (auto& cache : this->aim_cache)
	{
		if (!cache.points.empty())
			cache.points.clear();

		if (cache.best_point.filled)
			cache.best_point.reset();

		if (cache.player)
			cache.player = nullptr;
	}

	if (!g_ctx.round_start)
		g_ctx.round_start = true;

	if (g_cfg.visuals.eventlog.logs & 4)
		g_event_logger->add_message(xor_str("===============> ROUND STARTED <==============="), -1, true);

	this->shots.clear();
}

void c_rage_bot::on_game_events(c_game_event* event)
{
	if (!g_ctx.in_game)
		return;

	auto name = HASH_RT(event->get_name());

	switch (name)
	{
	case HASH("weapon_fire"):
		this->weapon_fire(event);
		break;
	case HASH("bullet_impact"):
		this->bullet_impact(event);
		break;
	case HASH("player_hurt"):
		this->player_hurt(event);
		break;
	case HASH("round_start"):
		this->round_start(event);
		break;
	}
}

void c_rage_bot::on_pre_predict()
{
	this->firing = false;

	if (this->shots.empty())
		return;

	auto time = math::ticks_to_time(g_ctx.tick_base - g_exploits->tickbase_offset());

	auto& shot = this->shots.front();
	if (std::abs(time - shot.time) > 1.f)
	{
		this->shots.erase(this->shots.begin());
		return;
	}

	if (shot.init_time != -1.f && shot.index && shot.damage == -1 && shot.fire && shot.impact_fire)
	{
		auto new_player = (c_csplayer*)interfaces::entity_list->get_entity(shot.index);

		if (new_player && new_player->is_player() && shot.record.ptr == new_player)
		{
			const auto studio_model = interfaces::model_info->get_studio_model(new_player->get_model());

			if (studio_model)
			{
				auto& resolver_info = shot.resolver;
				const auto end = shot.impact;
				if (!rage_tools::can_hit_hitbox(shot.start, end, shot.record.ptr, shot.hitbox, &shot.record))
				{
					float dist = shot.start.dist_to(shot.impact);
					float dist2 = shot.start.dist_to(shot.point);

					if (dist2 > dist)
						g_event_logger->add_message(xor_str("due to occlusion"), event_miss);
					else
						g_event_logger->add_message(xor_str("due to spread"), event_miss);
				}
				else
				{
					if (new_player->is_alive())
					{
						if (resolver_info.resolved)
							g_event_logger->add_message(xor_str("due to resolver"), event_miss);
						else
							g_event_logger->add_message(xor_str("due to unknown reason"), event_miss);

						this->missed_shots[shot.index]++;
					}
					else
						g_event_logger->add_message(xor_str("due to player death"), event_miss);
				}
			}
		}

		this->shots.erase(this->shots.begin());
	}
}
