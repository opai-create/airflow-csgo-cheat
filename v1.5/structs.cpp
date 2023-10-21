#include "globals.hpp"
#include "server_bones.hpp"

#pragma region ENGINETRACE
#define TRACE_FILTER_SIMPLE_VTABLE *offsets::trace_filter.add(XORN(0x3D)).cast<std::uintptr_t*>()
#define TRACE_FILTER_SKIP_TWO_ENTITIES_VTABLE *offsets::trace_filter_skip_entities.add(XORN(0x3)).cast<std::uintptr_t*>()

bool c_trace_filter::should_hit_entity(c_base_entity* entity, int mask)
{
    auto id = entity->get_client_class();
    if (id && std::strcmp(ignore, ""))
    {
        if (id->network_name == ignore)
            return false;
    }

    return !(entity == skip);
}

c_trace_filter_simple::c_trace_filter_simple() :
    vtable(TRACE_FILTER_SIMPLE_VTABLE) {}

c_trace_filter_simple::c_trace_filter_simple(c_base_entity* ignore_entity,
    const int collision_group, should_hit_fn should_hit) :

    vtable(TRACE_FILTER_SIMPLE_VTABLE),
    ignore_entity(ignore_entity),
    collision_group(collision_group),
    should_hit(should_hit) {}

c_trace_filter_skip_two_entities::c_trace_filter_skip_two_entities() :
    vtable(TRACE_FILTER_SKIP_TWO_ENTITIES_VTABLE) {}

c_trace_filter_skip_two_entities::c_trace_filter_skip_two_entities(c_base_entity* first_ignore_entity,
    c_base_entity* second_ignore_entity, const int collision_group,
    should_hit_fn should_hit) :

    vtable(TRACE_FILTER_SKIP_TWO_ENTITIES_VTABLE),
    first_ignore_entity(first_ignore_entity),
    second_ignore_entity(second_ignore_entity),
    collision_group(collision_group),
    should_hit(should_hit) {}

bool c_game_trace::did_hit_world() const
{
    return entity == HACKS->entity_list->get_client_entity(0);
}

bool c_game_trace::did_hit_non_world_entity() const
{
    return entity != nullptr && !did_hit_world();
}

void c_engine_trace::trace_line(const vec3_t& src, const vec3_t& dst, int mask, c_base_entity* entity, int collision_group, c_game_trace* trace)
{
    c_trace_filter_simple filter(entity, collision_group);
    this->trace_ray(ray_t(src, dst), mask, (c_trace_filter*)(&filter), trace);
}

void c_engine_trace::trace_hull(const vec3_t& src, const vec3_t& dst, const vec3_t& mins, const vec3_t& maxs, int mask, c_base_entity* entity, int collision_group, c_game_trace* trace)
{
    c_trace_filter_simple filter(entity, collision_group);
    this->trace_ray(ray_t(src, dst, mins, maxs), mask, (c_trace_filter*)(&filter), trace);
}
#pragma endregion

#pragma region ENTITIES
void c_animation_state::create(c_cs_player* player)
{
    offsets::create_animstate.cast<void(__thiscall*)(c_animation_state*, c_cs_player*)>()(this, player);
}

void c_animation_state::update(const vec3_t& angle)
{
    offsets::update_animstate.cast<void(__vectorcall*)(c_animation_state*, void*, float, float, float, void*)>()(this, NULL, NULL, angle.y, angle.x, NULL);
}

void c_animation_state::reset()
{
    offsets::reset_animstate.cast<void(__thiscall*)(c_animation_state*)>()(this);
}

float c_animation_state::get_min_rotation()
{
    float speed_walk = std::max(0.f, std::min(this->speed_as_portion_of_walk_top_speed, 1.f));
    float speed_duck = std::max(0.f, std::min(this->speed_as_portion_of_crouch_top_speed, 1.f));

    float modifier = ((this->walk_run_transition * -0.30000001f) - 0.19999999f) * speed_walk + 1.f;

    if (this->anim_duck_amount > 0.0f)
        modifier += ((this->anim_duck_amount * speed_duck) * (0.5f - modifier));

    return this->aim_yaw_min * modifier;
}

float c_animation_state::get_max_rotation()
{
    float speed_walk = std::max(0.f, std::min(this->speed_as_portion_of_walk_top_speed, 1.f));
    float speed_duck = std::max(0.f, std::min(this->speed_as_portion_of_crouch_top_speed, 1.f));

    float modifier = ((this->walk_run_transition * -0.30000001f) - 0.19999999f) * speed_walk + 1.f;

    if (this->anim_duck_amount > 0.0f)
        modifier += ((this->anim_duck_amount * speed_duck) * (0.5f - modifier));

    return this->aim_yaw_max * modifier;
}

void c_animation_state::increment_layer_cycle(c_animation_layers* layer, bool loop) {
    if (std::fabsf(layer->playback_rate) <= 0.f)
        return;

    float current_cycle = layer->cycle;
    current_cycle += last_update_increment * layer->playback_rate;

    if (!loop && current_cycle >= 1) {
        current_cycle = 0.999f;
    }

    set_layer_cycle(layer, math::clamp_cycle(current_cycle));
}

void c_animation_state::increment_layer_weight(c_animation_layers* layer) {
    if (std::fabsf(layer->weight_delta_rate) <= 0.f)
        return;

    float current_weight = layer->weight;
    current_weight += last_update_increment * layer->weight_delta_rate;
    current_weight = std::clamp<float>(current_weight, 0, 1);

    set_layer_weight(layer, current_weight);
}

bool c_animation_state::is_layer_sequence_finished(c_animation_layers* layer, float time) {
    return (layer->cycle + (time * layer->playback_rate)) >= 1.f;
}

void c_animation_state::set_layer_cycle(c_animation_layers* layer, float_t cycle) {
    auto player = reinterpret_cast<c_cs_player*>(layer->owner);

  //  if (player && layer->cycle != cycle)
   //     player->invalidate_physics_recursive(8);

    layer->cycle = cycle;
}

void c_animation_state::set_layer_rate(c_animation_layers* layer, float rate) {
    if (layer)
        layer->playback_rate = rate;
}

void c_animation_state::set_layer_weight(c_animation_layers* layer, float weight) {
    auto player = reinterpret_cast<c_cs_player*>(layer->owner);

    //if (player && layer->weight != weight)
     //   player->invalidate_physics_recursive(16);

    layer->weight = std::clamp(weight, 0.f, 1.f);
}

void c_animation_state::set_layer_weight_rate(c_animation_layers* layer, float prev) {
    if (layer)
        layer->weight_delta_rate = (layer->weight - prev) / this->last_update_increment;
}

void c_animation_state::set_layer_sequence(c_animation_layers* layer, int activity)
{
    int sequence = this->select_sequence_from_activity_modifier(activity);
    if (sequence < 2)
        return;

    auto player = reinterpret_cast<c_cs_player*>(layer->owner);

    if (player) {
    //    if (player && layer->sequence != sequence)
          //  player->invalidate_physics_recursive(16);

        layer->sequence = sequence;
        layer->playback_rate = player->get_layer_sequence_cycle_rate(layer, sequence);

      //  if (player && layer->cycle != 0.f)
      //      player->invalidate_physics_recursive(8);

        layer->cycle = 0.f;

      //  if (player && layer->weight != 0.f)
         //   player->invalidate_physics_recursive(16);
////
        layer->weight = 0.f;
    }
}

float c_animation_state::get_layer_ideal_weight_from_seq_cycle(c_animation_layers* layer, int index) {
    auto player = reinterpret_cast<c_cs_player*>(this->player);

    auto hdr = player->get_studio_hdr();
    if (!hdr)
        return 0;

    auto& sequence_desc = hdr->get_sequence_desc(layer->sequence);

    float cycle = layer->cycle;
    if (cycle >= 0.999f)
        cycle = 1.f;

    float ease_in = sequence_desc.fadeintime;
    float ease_out = sequence_desc.fadeouttime;

    float ideal_weight = 0.f;
    if (ease_in > 0 && cycle < ease_in)
        ideal_weight = math::smoothstep_bounds(0, ease_in, cycle);
    else if (ease_out < 1 && cycle > ease_out)
        ideal_weight = math::smoothstep_bounds(1.0f, ease_out, cycle);

    if (ideal_weight < 0.0015f)
        return 0.f;

    return (std::clamp(ideal_weight, 0.f, 1.f));
}

void animstate_pose_param_cache_t::set_value(c_cs_player* player, float value) {
    auto idx = this->index;

    float pose = 0.f;
    value = player->studio_set_pose_parameter(idx, value, pose);
    player->pose_parameter()[idx] = pose;
}

float animstate_pose_param_cache_t::get_value(c_cs_player* player)
{
    auto idx = this->index;

    auto hdr = player->get_studio_hdr();
    if (!hdr)
        return 0.f;

    auto val = player->pose_parameter()[idx];
    return get_pose_parameter_value(hdr, idx, val);
}

int c_animation_state::select_sequence_from_activity_modifier(int iActivity)
{
    bool ducking = this->anim_duck_amount > 0.55f;
    bool running = this->speed_as_portion_of_walk_top_speed > 0.25f;

    int current_sequence = 0;
    switch (iActivity)
    {
    case ACT_CSGO_JUMP:
    {
        current_sequence = 15 + static_cast<int32_t>(running);
        if (ducking)
            current_sequence = 17 + static_cast<int32_t>(running);
    }
    break;

    case ACT_CSGO_ALIVE_LOOP:
    {
        current_sequence = 8;
        if (this->weapon_last != this->weapon)
            current_sequence = 9;
    }
    break;

    case ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING:
    {
        current_sequence = 5;
    }
    break;

    case ACT_CSGO_FALL:
    {
        current_sequence = 14;
    }
    break;

    case ACT_CSGO_IDLE_TURN_BALANCEADJUST:
    {
        current_sequence = 4;
    }
    break;

    case ACT_CSGO_LAND_LIGHT:
    {
        current_sequence = 20;
        if (running)
            current_sequence = 22;

        if (ducking)
        {
            current_sequence = 21;
            if (running)
                current_sequence = 19;
        }
    }
    break;

    case ACT_CSGO_LAND_HEAVY:
    {
        current_sequence = 23;
        if (running)
            current_sequence = 24;
    }
    break;

    case ACT_CSGO_CLIMB_LADDER:
    {
        current_sequence = 13;
    }
    break;

    default:
        break;
    }

    return current_sequence;
}

void c_animation_state::update_layer(c_animation_layers* layer, int sequence, float rate, float cycle, float weight, int index)
{
    static auto update_layer_order_preset = offsets::update_layer_order_preset.cast<void(__thiscall*)(void*, int, int)>();

    if (sequence > 1)
    {
        auto player = (c_cs_player*)this->player;

        if (layer->sequence != sequence)
             player->invalidate_physics_recursive(16);

        layer->sequence = sequence;
        layer->playback_rate = rate;

        float temp_weight = std::clamp(weight, 0.f, 1.f);
        float temp_cycle = std::clamp(cycle, 0.f, 1.f);

        if (layer->cycle != temp_cycle)
             player->invalidate_physics_recursive(8);

        layer->cycle = temp_cycle;

        float prev_weight = layer->weight;
        if (prev_weight != temp_weight && (prev_weight == 0.f || temp_weight == 0.f))
                 player->invalidate_physics_recursive(16);

        layer->weight = temp_weight;

        update_layer_order_preset(this, index, layer->sequence);
    }
}

#pragma endregion

#pragma region TRACE
bool c_trace_filter_no_npcs_or_player::should_hit_entity(c_base_entity* entity, int mask)
{
    return entity != skip && !entity->is_player();
}
#pragma endregion