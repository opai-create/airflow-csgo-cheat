#include "rage_tools.h"
#include "ragebot.h"
#include "autowall.h"
#include "engine_prediction.h"

#include "../features.h"

namespace cheat_tools
{
  constexpr int max_traces = 48;

  weapon_config_t zeus_config = { true, false, false, 40, 100, 100, 80, 80, 0, 0, 0, 1, 0, { } };

  weapon_config_t get_weapon_config( )
  {
    if( !g_ctx.local || !g_ctx.local->is_alive( ) )
      return { };

    if( !g_ctx.weapon )
      return { };

    if( g_cfg.rage.weapon [ auto_snipers ].enable && g_ctx.weapon->is_auto_sniper( ) )
      return g_cfg.rage.weapon [ auto_snipers ];
    else if( g_cfg.rage.weapon [ heavy_pistols ].enable && g_ctx.weapon->is_heavy_pistols( ) )
      return g_cfg.rage.weapon [ heavy_pistols ];
    else if( g_cfg.rage.weapon [ pistols ].enable && g_ctx.weapon->is_pistols( ) )
      return g_cfg.rage.weapon [ pistols ];
    else if( g_cfg.rage.weapon [ scout ].enable && g_ctx.weapon->item_definition_index( ) == weapon_ssg08 )
      return g_cfg.rage.weapon [ scout ];
    else if( g_cfg.rage.weapon [ awp ].enable && g_ctx.weapon->item_definition_index( ) == weapon_awp )
      return g_cfg.rage.weapon [ awp ];
    else if( g_ctx.weapon->is_taser( ) )
      return zeus_config;

    return g_cfg.rage.weapon [ global ];
  }

  std::string hitbox_to_string( int id )
  {
    switch( id )
    {
    case 0:
      return xor_c( "head" );
      break;
    case 1:
      return xor_c( "neck" );
      break;
    case 2:
      return xor_c( "pelvis" );
      break;
    case 3:
      return xor_c( "stomach" );
      break;
    case 4:
      return xor_c( "lower chest" );
      break;
    case 5:
      return xor_c( "chest" );
      break;
    case 6:
      return xor_c( "upper chest" );
      break;
    case 7:
      return xor_c( "right thigh" );
      break;
    case 8:
      return xor_c( "left thigh" );
      break;
    case 9:
      return xor_c( "right leg" );
      break;
    case 10:
      return xor_c( "left leg" );
      break;
    case 11:
      return xor_c( "left foot" );
      break;
    case 12:
      return xor_c( "right foot" );
      break;
    case 13:
      return xor_c( "right hand" );
      break;
    case 14:
      return xor_c( "left hand" );
      break;
    case 15:
      return xor_c( "right upper arm" );
      break;
    case 16:
      return xor_c( "right lower arm" );
      break;
    case 17:
      return xor_c( "left upper arm" );
      break;
    case 18:
      return xor_c( "left lower arm" );
      break;
    }
  }

  std::string hitgroup_to_string( int hitgroup )
  {
    switch( hitgroup )
    {
    case hitgroup_generic:
      return xor_c( "generic" );
      break;
    case hitgroup_head:
      return xor_c( "head" );
      break;
    case hitgroup_chest:
      return xor_c( "chest" );
      break;
    case hitgroup_stomach:
      return xor_c( "body" );
      break;
    case hitgroup_leftarm:
      return xor_c( "leftarm" );
      break;
    case hitgroup_rightarm:
      return xor_c( "rightarm" );
      break;
    case hitgroup_leftleg:
      return xor_c( "leftleg" );
      break;
    case hitgroup_rightleg:
      return xor_c( "rightleg" );
      break;
    case hitgroup_gear:
      return xor_c( "gear" );
      break;
    case hitgroup_neck:
      return xor_c( "neck" );
      break;
    default:
      return xor_c( "unknown" );
    }
  }

  int hitbox_to_hitgroup( int hitbox )
  {
    switch( hitbox )
    {
    case hitbox_head:
    case hitbox_neck:
      return hitgroup_head;
      break;
    case hitbox_pelvis:
    case hitbox_stomach:
      return hitgroup_stomach;
      break;
    case hitbox_lower_chest:
    case hitbox_chest:
    case hitbox_upper_chest:
      return hitgroup_chest;
      break;
    case hitbox_left_thigh:
    case hitbox_left_calf:
    case hitbox_left_foot:
      return hitgroup_leftleg;
      break;
    case hitbox_right_thigh:
    case hitbox_right_calf:
    case hitbox_right_foot:
      return hitgroup_rightleg;
      break;
    case hitbox_left_hand:
    case hitbox_left_upper_arm:
    case hitbox_left_forearm:
      return hitgroup_leftarm;
      break;
    case hitbox_right_hand:
    case hitbox_right_upper_arm:
    case hitbox_right_forearm:
      return hitgroup_rightarm;
      break;
    default:
      return hitgroup_generic;
      break;
    }
  }

  constexpr int total_seeds = 255;

  vector2d calc_spread_angle( int bullets, float recoil_index, int i )
  {
    auto index = g_ctx.weapon->item_definition_index( );

    math::random_seed( i + 1u );

    auto v1 = math::random_float( 0.f, 1.f );
    auto v2 = math::random_float( 0.f, M_PI * 2.f );

    float v3{ }, v4{ };
    if( cvars::weapon_accuracy_shotgun_spread_patterns->get_int( ) > 0 )
      func_ptrs::calc_shotgun_spread( index, 0, static_cast< int >( bullets * recoil_index ), &v4, &v3 );
    else
    {
      v3 = math::random_float( 0.f, 1.f );
      v4 = math::random_float( 0.f, M_PI * 2.f );
    }

    if( recoil_index < 3.f && index == weapon_negev )
    {
      for( auto i = 3; i > recoil_index; --i )
      {
        v1 *= v1;
        v3 *= v3;
      }

      v1 = 1.f - v1;
      v3 = 1.f - v3;
    }

    const auto inaccuracy = v1 * g_engine_prediction->predicted_inaccuracy;
    const auto spread = v3 * g_engine_prediction->predicted_spread;

    return { std::cos( v2 ) * inaccuracy + std::cos( v4 ) * spread, std::sin( v2 ) * inaccuracy + std::sin( v4 ) * spread };
  }

  bool can_hit_hitbox_wrap( const vector3d& start, const vector3d& end, c_csplayer* player, int hitbox, records_t* record, matrix3x4_t* matrix )
  {
    auto current_bones = matrix ? matrix : record->sim_orig.bone;
    auto model = player->get_model( );
    if( !model )
      return false;

    auto studio_model = interfaces::model_info->get_studio_model( player->get_model( ) );
    auto set = studio_model->get_hitbox_set( 0 );

    if( !set )
      return false;

    auto studio_box = set->get_hitbox( hitbox );
    if( !studio_box )
      return false;

    vector3d min{ }, max{ };

    math::vector_transform( studio_box->bbmin, current_bones [ studio_box->bone ], min );
    math::vector_transform( studio_box->bbmax, current_bones [ studio_box->bone ], max );

    if( studio_box->radius != 1.f )
      return math::segment_to_segment( start, end, min, max ) < studio_box->radius;

    math::vector_i_transform( start, current_bones [ studio_box->bone ], min );
    math::vector_i_transform( end, current_bones [ studio_box->bone ], max );
    return math::intersect_line_with_bb( min, max, studio_box->bbmin, studio_box->bbmax );
  }

  bool can_hit_hitbox( const vector3d& start, const vector3d& end, c_csplayer* player, int hitbox, records_t* record, matrix3x4_t* matrix )
  {
    return can_hit_hitbox_wrap( start, end, player, hitbox, record, matrix );
  }

  bool is_accuracy_valid( c_csplayer* player, point_t& point, float amount, float* out_chance )
  {
#ifdef _DEBUG
    spread_point.reset( );
    current_spread = 0.f;
    spread_points.clear( );
#endif

    if( cvars::weapon_accuracy_nospread->get_int( ) > 0 || amount <= 0.f )
      return true;

    auto model = interfaces::model_info->get_studio_model( player->get_model( ) );
    if( !model )
      return false;

    auto set = model->get_hitbox_set( player->hitbox_set( ) );

    if( !set )
      return false;

    auto studio_box = set->get_hitbox( point.hitbox );
    if( !studio_box )
      return false;

    auto weapon = g_ctx.weapon;
    if( !weapon )
      return false;

    auto weapon_info = weapon->get_weapon_info( );
    if( !weapon_info )
      return false;

    auto state = g_ctx.local->animstate( );
    if( !state )
      return false;

    auto range = weapon_info->range;

    vector3d forward, right, up;
    vector3d start = g_ctx.local->get_eye_position( );
    vector3d pos = math::angle_from_vectors( start, point.position );
    math::angle_to_vectors( pos, forward, right, up );
   
 /*   if( point.center )
    {
      if( g_ctx.spread > 0.f && g_ctx.spread <= g_ctx.ideal_spread && ( g_ctx.spread / g_ctx.ideal_spread ) >= amount )
        ++point.chance_ticks;
    }

    if( point.chance_ticks >= 5 )
    {
      *out_chance = amount;
      point.chance_ticks = 0;
      return true;
    }*/


#ifdef _DEBUG
    if( debug_hitchance )
    {
      current_spread = g_ctx.spread;
      g_render->world_to_screen( point.position, spread_point );
    }
#endif

    int hits = 0;
    for( int i = 0; i < total_seeds; ++i )
    {
      auto spread_angle = calc_spread_angle( weapon_info->bullets, weapon->recoil_index( ), i );

      auto direction = forward + ( right * spread_angle.x ) + ( up * spread_angle.y );
      direction = direction.normalized( );

      auto end = start + direction * range;
#ifdef _DEBUG
      if( debug_hitchance )
      {
        vector2d scr_end;
        if( g_render->world_to_screen( end, scr_end ) )
          spread_points.emplace_back( scr_end );
      }
#endif

      if( can_hit_hitbox( start, end, player, point.hitbox, point.record ) )
        ++hits;

      if( ( float )( hits + total_seeds - i ) / ( float )( total_seeds ) < amount )
        return false;
    }

    *out_chance = ( float )hits / ( float )total_seeds;
    return ( ( float )hits / ( float )total_seeds ) >= amount;
  }

  int get_legit_tab( c_basecombatweapon* temp_weapon )
  {
    auto weapon = temp_weapon ? temp_weapon : g_ctx.weapon;
    if( !weapon )
      return 0;

    if( weapon->is_knife( ) )
      return weapon_cfg_knife;

    auto idx = weapon->item_definition_index( );

    switch( idx )
    {
    case weapon_deagle:
      return weapon_cfg_deagle;
      break;
    case weapon_elite:
      return weapon_cfg_duals;
      break;
    case weapon_fiveseven:
      return weapon_cfg_fiveseven;
      break;
    case weapon_glock:
      return weapon_cfg_glock;
      break;
    case weapon_ak47:
      return weapon_cfg_ak47;
      break;
    case weapon_aug:
      return weapon_cfg_aug;
      break;
    case weapon_awp:
      return weapon_cfg_awp;
      break;
    case weapon_famas:
      return weapon_cfg_famas;
      break;
    case weapon_g3sg1:
      return weapon_cfg_g3sg1;
      break;
    case weapon_galilar:
      return weapon_cfg_galil;
      break;
    case weapon_m249:
      return weapon_cfg_m249;
      break;
    case weapon_m4a1:
      return weapon_cfg_m4a1;
      break;
    case weapon_mac10:
      return weapon_cfg_mac10;
      break;
    case weapon_p90:
      return weapon_cfg_p90;
      break;
    case weapon_mp5sd:
      return weapon_cfg_mp5sd;
      break;
    case weapon_ump45:
      return weapon_cfg_ump45;
      break;
    case weapon_xm1014:
      return weapon_cfg_xm1014;
      break;
    case weapon_bizon:
      return weapon_cfg_bizon;
      break;
    case weapon_mag7:
      return weapon_cfg_mag7;
      break;
    case weapon_negev:
      return weapon_cfg_negev;
      break;
    case weapon_sawedoff:
      return weapon_cfg_sawedoff;
      break;
    case weapon_tec9:
      return weapon_cfg_tec9;
      break;
    case weapon_hkp2000:
      return weapon_cfg_p2000;
      break;
    case weapon_mp7:
      return weapon_cfg_mp7;
      break;
    case weapon_mp9:
      return weapon_cfg_mp9;
      break;
    case weapon_nova:
      return weapon_cfg_nova;
      break;
    case weapon_p250:
      return weapon_cfg_p250;
      break;
    case weapon_scar20:
      return weapon_cfg_scar20;
      break;
    case weapon_sg556:
      return weapon_cfg_sg556;
      break;
    case weapon_ssg08:
      return weapon_cfg_ssg08;
      break;
    case weapon_m4a1_silencer:
      return weapon_cfg_m4a1s;
      break;
    case weapon_usp_silencer:
      return weapon_cfg_usps;
      break;
    case weapon_cz75a:
      return weapon_cfg_cz75;
      break;
    case weapon_revolver:
      return weapon_cfg_revolver;
      break;
    default:
      return 0;
      break;
    }
  }

  skin_weapon_t get_skin_weapon_config( )
  {
    if( !g_ctx.local || !g_ctx.local->is_alive( ) )
      return { };

    if( !g_ctx.weapon )
      return { };

    int tab = get_legit_tab( );
    return g_cfg.skins.skin_weapon [ tab ];
  }

  std::vector< std::pair< vector3d, bool > > get_multipoints( c_csplayer* player, int hitbox, matrix3x4_t* matrix )
  {
    std::vector< std::pair< vector3d, bool > > points = { };

    auto model = player->get_model( );
    if( !model )
      return points;

    auto hdr = interfaces::model_info->get_studio_model( model );
    if( !hdr )
      return points;

    auto set = hdr->get_hitbox_set( 0 );
    if( !set )
      return points;

    auto bbox = set->get_hitbox( hitbox );
    if( !bbox )
      return points;

    if( bbox->radius <= 0.f )
    {
      matrix3x4_t rot_matrix = { };
      rot_matrix.angle_matrix( bbox->rotation );

      matrix3x4_t mat = { };
      math::contact_transforms( matrix [ bbox->bone ], rot_matrix, mat );

      vector3d origin = mat.get_origin( );

      vector3d center = ( bbox->bbmin + bbox->bbmax ) * 0.5f;

      if( hitbox == hitbox_left_foot || hitbox == hitbox_right_foot )
        points.emplace_back( center, true );

      if( points.empty( ) )
        return points;

      for( auto& p : points )
      {
        p.first = { p.first.dot( mat.mat [ 0 ] ), p.first.dot( mat.mat [ 1 ] ), p.first.dot( mat.mat [ 2 ] ) };
        p.first += origin;
      }
    }
    else
    {
      vector3d max = bbox->bbmax;
      vector3d min = bbox->bbmin;
      vector3d center = ( bbox->bbmin + bbox->bbmax ) * 0.5f;

      float head_slider = get_weapon_config( ).scale_head / 100.f;
      float body_slider = get_weapon_config( ).scale_body / 100.f;

      float head_scale = bbox->radius * head_slider;
      float body_scale = bbox->radius * body_slider;

      constexpr float rotation = 0.70710678f;
      float near_center_scale = bbox->radius * ( head_slider / 2.f );

      if( hitbox == hitbox_head )
      {
        points.emplace_back( center, true );

        vector3d point{ };
        point = { max.x + 0.70710678f * head_scale, max.y - 0.70710678f * head_scale, max.z };
        points.emplace_back( point, false );

        point = { max.x, max.y, max.z + head_scale };
        points.emplace_back( point, false );

        point = { max.x, max.y, max.z - head_scale };
        points.emplace_back( point, false );

        point = { max.x, max.y - head_scale, max.z };
        points.emplace_back( point, false );
      }
      else
      {
        if( hitbox == hitbox_stomach )
        {
          points.emplace_back( center, true );
          points.emplace_back( vector3d( center.x, center.y, min.z + body_scale ), false );
          points.emplace_back( vector3d( center.x, center.y, max.z - body_scale ), false );
          points.emplace_back( vector3d{ center.x, max.y - body_scale, center.z }, false );
        }
        else if( hitbox == hitbox_pelvis || hitbox == hitbox_upper_chest )
        {
          points.emplace_back( center, true );
          points.emplace_back( vector3d( center.x, center.y, max.z + body_scale ), false );
          points.emplace_back( vector3d( center.x, center.y, min.z - body_scale ), false );
        }
        else if( hitbox == hitbox_lower_chest || hitbox == hitbox_chest )
        {
          points.emplace_back( center, true );
          points.emplace_back( vector3d( center.x, center.y, max.z + body_scale ), false );
          points.emplace_back( vector3d( center.x, center.y, min.z - body_scale ), false );

          points.emplace_back( vector3d{ center.x, max.y - body_scale, center.z }, false );
        }
        else if( hitbox == hitbox_right_calf || hitbox == hitbox_left_calf )
        {
          points.emplace_back( center, true );
          points.emplace_back( vector3d{ max.x - ( bbox->radius / 2.f ), max.y, max.z }, false );
        }
        else if( hitbox == hitbox_right_thigh || hitbox == hitbox_left_thigh )
        {
          points.emplace_back( center, true );
        }
        else if( hitbox == hitbox_right_upper_arm || hitbox == hitbox_left_upper_arm )
        {
          points.emplace_back( vector3d{ max.x + bbox->radius, center.y, center.z }, false );
        }
        else
          points.emplace_back( center, true );
      }

      if( points.empty( ) )
        return points;

      for( auto& p : points )
        math::vector_transform( p.first, matrix [ bbox->bone ], p.first );
    }

    return points;
  }
}

namespace resolver
{
  // reference:
  // https://github.com/cybergodjsp/bameware-source/blob/master/FEATURES/Resolver.cpp#L74
  __forceinline void store_freestanding( c_csplayer* player, records_t* current )
  {
    auto& resolver_info = info [ player->index( ) ];

    auto& freestanding = resolver_info.freestanding;
    freestanding.available = false;

    if( current->velocity.length( true ) > 0.1f && !current->fake_walking )
      return;

    float at_target = math::normalize( math::angle_from_vectors( g_ctx.local->origin( ), player->origin( ) ).y );

    const float height = 64.f;

    vector3d direction_1, direction_2;
    math::angle_to_vectors( vector3d( 0.f, at_target - 90.f, 0.f ), direction_1 );
    math::angle_to_vectors( vector3d( 0.f, at_target + 90.f, 0.f ), direction_2 );

    const auto left_eye_pos = player->origin( ) + vector3d( 0, 0, height ) + ( direction_1 * 16.f );
    const auto right_eye_pos = player->origin( ) + vector3d( 0, 0, height ) + ( direction_2 * 16.f );

    freestanding.left_damage = g_auto_wall->fire_bullet( g_ctx.local, player, g_ctx.weapon_info, g_ctx.weapon->is_taser( ), g_ctx.eye_position, left_eye_pos ).dmg;

    freestanding.right_damage = g_auto_wall->fire_bullet( g_ctx.local, player, g_ctx.weapon_info, g_ctx.weapon->is_taser( ), g_ctx.eye_position, right_eye_pos ).dmg;

    c_game_trace trace = { };
    c_trace_filter_world_only filter = { };

    interfaces::engine_trace->trace_ray( ray_t( left_eye_pos, g_ctx.eye_position ), mask_all, &filter, &trace );
    freestanding.left_fraction = trace.fraction;

    interfaces::engine_trace->trace_ray( ray_t( right_eye_pos, g_ctx.eye_position ), mask_all, &filter, &trace );
    freestanding.right_fraction = trace.fraction;

    freestanding.available = true;
  }

  void start( c_csplayer* player, records_t* current )
  {
    auto& resolver_info = info [ player->index( ) ];
    resolver_info.valid = false;

    if( !g_cfg.rage.enable || !g_ctx.local->is_alive( ) )
    {
      if( resolver_info.valid )
        resolver_info.reset( );

      return;
    }

    if( player->is_bot( ) )
      return;

    auto state = player->animstate( );
    if( !state )
      return;

    store_freestanding( player, current );

    float at_target = math::normalize( math::angle_from_vectors( g_ctx.local->origin( ), player->origin( ) ).y );
    int misses = g_rage_bot->missed_shots [ player->index( ) ];

    auto& lby_data = resolver_info.lby;

    if( !current->on_ground )
    {
      if( misses < 1 )
      {
        player->eye_angles( ).y = at_target;
        resolver_info.mode = xor_str( "air" );
      }
      else
      {
        float velyaw = math::rad_to_deg( std::atan2( current->velocity.y, current->velocity.x ) );

        switch( misses % 3 )
        {
        case 1:
          player->eye_angles( ).y = velyaw + 180.f;
          break;
        case 2:
          player->eye_angles( ).y = velyaw - 90.f;
          break;
        case 0:
          player->eye_angles( ).y = velyaw + 90.f;
          break;
        }

        resolver_info.mode = xor_str( "air brute" );
      }

      float lby = player->lby( );

      lby_data.lby_time = current->sim_time;
      lby_data.old_lby = lby;
    }
    else
    {
      if( current->velocity.length( true ) > 0.1f && !current->fake_walking )
      {
        float lby = player->lby( );

        player->eye_angles( ).y = lby;

        lby_data.lby_time = current->sim_time + 0.22f;
        lby_data.old_lby = lby;

        resolver_info.last_moving_lby = lby;
        resolver_info.mode = xor_str( "moving" );
      }
      else
      {
        float lby = player->lby( );
        if( lby_data.old_lby != lby )
        {
          player->eye_angles( ).y = lby;
          resolver_info.mode = xor_str( "lby update" );
          lby_data.lby_time = current->sim_time + 1.1f;
          lby_data.old_lby = lby;
          return;
        }
        else if( current->sim_time >= lby_data.lby_time )
        {
          player->eye_angles( ).y = lby;
          resolver_info.mode = xor_str( "lby update" );
          lby_data.lby_time = current->sim_time + 1.1f;
          return;
        }

        if( misses > 0 )
        {
          switch( misses % 3 )
          {
          case 1:
            player->eye_angles( ).y = at_target;
            break;
          case 2:
            player->eye_angles( ).y = at_target - 90.f;
            break;
          case 0:
            player->eye_angles( ).y = at_target + 90.f;
            break;
          }

          resolver_info.mode = xor_str( "stand brute" );
        }
        else
        {
          auto& freestanding = resolver_info.freestanding;
          if( freestanding.available )
          {
            if( freestanding.left_damage <= 0 && freestanding.right_damage <= 0 )
            {
              if( freestanding.right_fraction < freestanding.left_fraction )
                freestanding.yaw = at_target + 90.f;
              else if( freestanding.right_fraction > freestanding.left_fraction )
                freestanding.yaw = at_target - 90.f;
            }
            else
            {
              if( freestanding.left_damage > freestanding.right_damage )
                freestanding.yaw = at_target + 90.f;
              else if( freestanding.left_damage < freestanding.right_damage )
                freestanding.yaw = at_target - 90.f;
            }

            player->eye_angles( ).y = math::normalize( freestanding.yaw );

            resolver_info.mode = xor_str( "wall dtc" );
          }
          else
          {
            player->eye_angles( ).y = resolver_info.last_moving_lby;
            resolver_info.mode = xor_str( "last moving lby" );
          }

          resolver_info.last_stand_angle = player->eye_angles( ).y;
        }
      }
    }

    player->eye_angles( ).y = math::normalize( player->eye_angles( ).y );

    resolver_info.valid = true;
  }

  void on_fsn( )
  {
    static bool should_clear = true;
    if( g_ctx.in_game )
    {
      should_clear = true;
      return;
    }

    if( should_clear )
    {
      for( auto& i : info )
        i.reset( );

      should_clear = false;
    }
  }
}