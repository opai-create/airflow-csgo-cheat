#include "autowall.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../../base/sdk/entity.h"

constexpr int shot_hull = mask_shot_hull;
constexpr int shot_player = mask_shot_hull | contents_hitbox;

class c_trace_filter_simple
{
public:
  __forceinline c_trace_filter_simple( ): vtable{ *patterns::trace_filter.add( 0x3D ).as< uintptr_t* >( ) }
  {
  }

  __forceinline c_trace_filter_simple( c_baseentity* const ignore_entity, const int collision_group )
      : vtable{ *patterns::trace_filter.add( 0x3D ).as< uintptr_t* >( ) }, ignore_entity{ ignore_entity }, collision_group{ collision_group }
  {
  }

  uintptr_t vtable{ };
  c_baseentity* ignore_entity{ };
  int collision_group{ };
  should_hit_fn_t should_hit_fn{ };
};

class c_trace_filter_skip_two_entities
{
public:
  __forceinline c_trace_filter_skip_two_entities( ): vtable{ *patterns::trace_filter_skip_entities.add( 0x59 ).as< uintptr_t* >( ) }
  {
  }

  __forceinline c_trace_filter_skip_two_entities( c_baseentity* const ignore_entity0, c_baseentity* const ignore_entity1, const int collision_group = 0 )
      : vtable{ *patterns::trace_filter_skip_entities.add( 0x59 ).as< uintptr_t* >( ) }, ignore_entity0{ ignore_entity0 }, collision_group{ collision_group }, ignore_entity1{ ignore_entity1 }
  {
  }

  uintptr_t vtable{ };
  c_baseentity* ignore_entity0{ };
  int collision_group{ };
  should_hit_fn_t should_hit_fn{ };
  c_baseentity* ignore_entity1{ };
};

bool c_auto_wall::is_breakable_entity( c_baseentity* entity )
{
  auto client_class = entity->get_client_class( );
  if( !client_class )
    return func_ptrs::is_breakable_entity( entity );

  // on v4c map cheat shoots through this
  // ignore these fucks
  if( client_class->class_id == CBaseButton || client_class->class_id == CPhysicsProp )
    return false;

  // check if it's window by name (pasted from onetap)
  auto v3 = ( int )client_class->network_name;
  if( *( DWORD* )v3 == 0x65724243 )
  {
    if( *( DWORD* )( v3 + 7 ) == 0x53656C62 )
      return 1;
  }

  if( *( DWORD* )v3 == 0x73614243 )
  {
    if( *( DWORD* )( v3 + 7 ) == 0x79746974 )
      return 1;
  }

  // if it's proper entity, use original func then
  return func_ptrs::is_breakable_entity( entity );
}

bool c_auto_wall::can_hit_point( c_csplayer* entity, const vector3d& point, const vector3d& source, int min_damage, c_csplayer* shooter, int* out )
{
  auto weapon = shooter->get_active_weapon( );
  if( !weapon )
    return false;

  auto weapon_info = weapon->get_weapon_info( );
  if( !weapon_info )
    return false;

  const auto origin_backup = shooter->get_abs_origin( );

  shooter->set_abs_origin( vector3d( source.x, source.y, origin_backup.z ) );

  const auto& data = this->fire_bullet( shooter, entity, weapon_info, weapon->is_taser( ), source, point );

  // interfaces::debug_overlay->add_text_overlay(point, interfaces::global_vars->interval_per_tick * 2.f, "%d", data.dmg);

  shooter->set_abs_origin( origin_backup );

  if( out )
    *out = data.dmg;

  return data.dmg >= min_damage + 1;
}

bool c_auto_wall::trace_to_exit( const vector3d& src, const vector3d& dir, const c_game_trace& enter_trace, c_game_trace& exit_trace )
{
  float dist{ };
  int first_contents{ };

  constexpr auto k_step_size = 4.f;
  constexpr auto k_max_dist = 90.f;

  while( dist <= k_max_dist )
  {
    dist += k_step_size;

    const auto out = src + ( dir * dist );

    const auto cur_contents = interfaces::engine_trace->get_point_contents( out, shot_player );

    if( !first_contents )
      first_contents = cur_contents;

    if( cur_contents & shot_hull && ( !( cur_contents & contents_hitbox ) || cur_contents == first_contents ) )
      continue;

    interfaces::engine_trace->trace_ray( ray_t( out, out - dir * k_step_size ), shot_player, nullptr, &exit_trace );

    if( exit_trace.start_solid && exit_trace.surface.flags & surf_hitbox )
    {
      c_trace_filter_simple trace_filter{ exit_trace.entity, 0 };

      interfaces::engine_trace->trace_ray( { out, src }, shot_hull, ( i_trace_filter* )( &trace_filter ), &exit_trace );

      if( exit_trace.did_hit( ) && !exit_trace.start_solid )
        return true;

      continue;
    }

    if( !exit_trace.did_hit( ) || exit_trace.start_solid )
    {
      if( enter_trace.entity && enter_trace.entity->index( ) && this->is_breakable_entity( enter_trace.entity ) )
      {
        exit_trace = enter_trace;
        exit_trace.end = src + dir;

        return true;
      }

      continue;
    }

    if( exit_trace.surface.flags & surf_nodraw )
    {
      if( this->is_breakable_entity( exit_trace.entity ) && this->is_breakable_entity( enter_trace.entity ) )
        return true;

      if( !( enter_trace.surface.flags & surf_nodraw ) )
        continue;
    }

    if( exit_trace.plane.normal.dot( dir ) <= 1.f )
      return true;
  }

  return false;
}

void c_auto_wall::clip_trace_to_player( const vector3d& src, const vector3d& dst, c_game_trace& trace, c_csplayer* const player )
{
  vector3d mins = player->bb_mins( ), maxs = player->bb_maxs( );

  vector3d dir( dst - src );
  dir = dir.normalized( );

  vector3d center = ( maxs + mins ) / 2, pos( center + player->origin( ) );

  vector3d to = pos - src;
  float range_along = dir.dot( to );

  float range;

  if( range_along < 0.f )
    range = -to.length( false );
  else if( range_along > dir.length( false ) )
    range = -( pos - dst ).length( false );
  else
  {
    auto ray( pos - ( dir * range_along + src ) );
    range = ray.length( false );
  }

  if( range <= 60.f )
  {
    c_game_trace new_trace{ };
    interfaces::engine_trace->clip_ray_to_entity( ray_t( src, dst ), shot_player, player, &new_trace );

    if( trace.fraction > new_trace.fraction )
      trace = new_trace;
  }
}

bool c_auto_wall::handle_bullet_penetration(
  c_csplayer* const shooter, const weapon_info_t* const wpn_data, const c_game_trace& enter_trace, vector3d& src, const vector3d& dir, int& pen_count, float& cur_dmg, const float pen_modifier )
{
  if( pen_count <= 0 || wpn_data->penetration <= 0.f )
    return false;

  c_game_trace exit_trace{ };

  if( !this->trace_to_exit( enter_trace.end, dir, enter_trace, exit_trace ) && !( interfaces::engine_trace->get_point_contents( enter_trace.end, shot_hull ) & shot_hull ) )
    return false;

  auto final_dmg_modifier = 0.16f;
  float combined_pen_modifier{ };

  const auto exit_surface_data = interfaces::phys_surface_props->get_surface_data( exit_trace.surface.surface_props );
  const auto enter_surface_data = interfaces::phys_surface_props->get_surface_data( enter_trace.surface.surface_props );

  if( enter_surface_data->game.material == 'G' || enter_surface_data->game.material == 'Y' )
  {
    final_dmg_modifier = 0.05f;
    combined_pen_modifier = 3.f;
  }
  else if( enter_trace.contents & contents_grate || enter_trace.surface.flags & surf_nodraw )
  {
    final_dmg_modifier = 0.16f;
    combined_pen_modifier = 1.f;
  }
  else if( enter_trace.entity && cvars::ff_damage_reduction_bullets->get_float( ) == 0.f && enter_surface_data->game.material == 'F' && ( ( c_csplayer* )enter_trace.entity )->team( ) == shooter->team( ) )
  {
    const auto dmg_bullet_pen = cvars::ff_damage_bullet_penetration->get_float( );
    if( dmg_bullet_pen == 0.f )
      return false;

    combined_pen_modifier = dmg_bullet_pen;
    final_dmg_modifier = 0.16f;
  }
  else
  {
    combined_pen_modifier = ( enter_surface_data->game.penetration_modifier + exit_surface_data->game.penetration_modifier ) * 0.5f;

    final_dmg_modifier = 0.16f;
  }

  if( enter_surface_data->game.material == exit_surface_data->game.material )
  {
    if( exit_surface_data->game.material == 'U' || exit_surface_data->game.material == 'W' )
      combined_pen_modifier = 3.f;
    else if( exit_surface_data->game.material == 'L' )
      combined_pen_modifier = 2.f;
  }

  const auto modifier = std::max( 1.f / combined_pen_modifier, 0.f );
  const auto pen_dist = ( exit_trace.end - enter_trace.end ).length_sqr( );

  const auto lost_dmg = cur_dmg * final_dmg_modifier + pen_modifier * ( modifier * 3.f ) + ( pen_dist * modifier ) / 24.f;

  if( lost_dmg > cur_dmg )
    return false;

  if( lost_dmg > 0.f )
    cur_dmg -= lost_dmg;

  if( cur_dmg < 1.f )
    return false;

  --pen_count;

  src = exit_trace.end;

  return true;
}

void c_auto_wall::scale_dmg( c_csplayer* const player, float& dmg, const float armor_ratio, const float headshot_mult, const int hitgroup )
{
  const auto has_heavy_armor = player->has_heavy_armor( );

  switch( hitgroup )
  {
  case 1:
    dmg *= 4.f;

    if( has_heavy_armor )
      dmg *= 0.5f;

    break;
  case 3:
    dmg *= 1.25f;
    break;
  case 6:
  case 7:
    dmg *= 0.75f;

    break;
  }

  const auto armor_value = player->armor_value( );
  if( !armor_value || hitgroup < 0 || hitgroup > 5 || ( hitgroup == 1 && !player->has_helmet( ) ) )
    return;

  auto heavy_ratio = 1.f, bonus_ratio = 0.5f, ratio = armor_ratio * 0.5f;

  if( has_heavy_armor )
  {
    ratio *= 0.2f;
    heavy_ratio = 0.25f;
    bonus_ratio = 0.33f;
  }

  auto dmg_to_hp = dmg * ratio;

  if( ( ( dmg - dmg_to_hp ) * ( bonus_ratio * heavy_ratio ) ) > armor_value )
    dmg -= armor_value / bonus_ratio;
  else
    dmg = dmg_to_hp;
}

pen_data_t c_auto_wall::fire_bullet( c_csplayer* const shooter, c_csplayer* const target, const weapon_info_t* const wpn_data, const bool is_taser, vector3d src, const vector3d& dst, bool ignore_target )
{
  auto pen_modifier = std::max( ( 3.f / wpn_data->penetration ) * 1.25f, 0.f );

  float cur_dist{ };
  pen_data_t data{ };

  data.remaining_pen = 4;

  auto cur_dmg = static_cast< float >( wpn_data->dmg );
  auto dir = dst - src;
  dir = dir.normalized( );

  c_game_trace trace{ };
  c_trace_filter_skip_two_entities trace_filter{ };
  c_csplayer* last_hit_player{ };

  auto max_dist = wpn_data->range;

  while( cur_dmg > 0.f )
  {
    max_dist -= cur_dist;

    const auto cur_dst = src + dir * max_dist;

    trace_filter.ignore_entity0 = shooter;
    trace_filter.ignore_entity1 = last_hit_player;

    interfaces::engine_trace->trace_ray( ray_t( src, cur_dst ), shot_player, ( i_trace_filter* )( &trace_filter ), &trace );

    // interfaces::debug_overlay->add_text_overlay(cur_dst, interfaces::global_vars->interval_per_tick * 2.f, "%s", trace.surface.name);

    if( target )
      this->clip_trace_to_player( src, cur_dst + dir * 40.f, trace, target );

    if( trace.fraction == 1.f )
      break;

    cur_dist += trace.fraction * max_dist;
    cur_dmg *= std::pow( wpn_data->range_modifier, cur_dist / 500.f );

    if( target )
    {
      if( trace.entity )
      {
        const auto is_player = trace.entity->is_player( );
        if( ( trace.entity == target || ( is_player && ( ( c_csplayer* )trace.entity )->team( ) != shooter->team( ) ) ) && ( ( trace.hitgroup >= 1 && trace.hitgroup <= 7 ) || trace.hitgroup == 10 ) )
        {
          data.hit_player = ( c_csplayer* )trace.entity;
          data.hitbox = trace.hitbox;
          data.hitgroup = trace.hitgroup;

          if( is_taser )
            data.hitgroup = 0;

          this->scale_dmg( data.hit_player, cur_dmg, wpn_data->armor_ratio, wpn_data->crosshair_delta_distance, data.hitgroup );

          data.dmg = static_cast< int >( cur_dmg );

          return data;
        }

        last_hit_player = is_player ? ( c_csplayer* )trace.entity : nullptr;
      }
      else
        last_hit_player = nullptr;
    }

    if( is_taser || ( cur_dist > 3000.f && wpn_data->penetration > 0.f ) )
      break;

    const auto enter_surface = interfaces::phys_surface_props->get_surface_data( trace.surface.surface_props );
    if( enter_surface->game.penetration_modifier < 0.1f || !handle_bullet_penetration( shooter, wpn_data, trace, src, dir, data.remaining_pen, cur_dmg, pen_modifier ) )
      break;

    if( ignore_target )
      data.dmg = static_cast< int >( cur_dmg );
  }

  return data;
}

pen_data_t c_auto_wall::fire_emulated( c_csplayer* const shooter, c_csplayer* const target, vector3d src, const vector3d& dst )
{
  static const auto wpn_data = []( )
  {
    weapon_info_t wpn_data{ };

    wpn_data.dmg = 115;
    wpn_data.range = 8192.f;
    wpn_data.penetration = 2.5f;
    wpn_data.range_modifier = 0.99f;
    wpn_data.armor_ratio = 1.95f;

    return wpn_data;
  }( );

  const auto pen_modifier = std::max( ( 3.f / wpn_data.penetration ) * 1.25f, 0.f );

  float cur_dist{ };

  pen_data_t data{ };

  data.remaining_pen = 4;

  auto cur_dmg = static_cast< float >( wpn_data.dmg );

  auto dir = dst - src;

  const auto max_dist = dir.normalized_float( );

  c_game_trace trace{ };
  c_trace_filter_skip_two_entities trace_filter{ };

  c_csplayer* last_hit_player{ };

  while( cur_dmg > 0.f )
  {
    const auto dist_remaining = wpn_data.range - cur_dist;

    const auto cur_dst = src + dir * dist_remaining;

    trace_filter.ignore_entity0 = shooter;
    trace_filter.ignore_entity1 = last_hit_player;

    interfaces::engine_trace->trace_ray( ray_t( src, cur_dst ), shot_player, ( i_trace_filter* )( &trace_filter ), &trace );

    // interfaces::debug_overlay->add_line_overlay(trace.start, trace.end, 255, 255, 255, false, 0.1f);

    if( target )
      this->clip_trace_to_player( src, cur_dst + dir * 40.f, trace, target );

    if( trace.fraction == 1.f || ( trace.end - src ).length( false ) > max_dist )
      break;

    cur_dist += trace.fraction * dist_remaining;
    cur_dmg *= std::pow( wpn_data.range_modifier, cur_dist / 500.f );

    if( trace.entity )
    {
      const auto is_player = trace.entity->is_player( );
      if( trace.entity == target )
      {
        data.hit_player = static_cast< c_csplayer* >( trace.entity );
        data.hitbox = trace.hitbox;
        data.hitgroup = trace.hitgroup;
        data.dmg = static_cast< int >( cur_dmg );

        return data;
      }

      last_hit_player = is_player ? static_cast< c_csplayer* >( trace.entity ) : nullptr;
    }
    else
      last_hit_player = nullptr;

    if( cur_dist > 3000.f && wpn_data.penetration > 0.f )
      break;

    const auto enter_surface = interfaces::phys_surface_props->get_surface_data( trace.surface.surface_props );
    if( enter_surface->game.penetration_modifier < 0.1f || !handle_bullet_penetration( shooter, &wpn_data, trace, src, dir, data.remaining_pen, cur_dmg, pen_modifier ) )
      break;
  }

  data.dmg = static_cast< int >( cur_dmg );

  return data;
}