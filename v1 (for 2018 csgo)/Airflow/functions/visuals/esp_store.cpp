#include "esp_store.h"

#include "../config_vars.h"
#include "../features.h"

#include "../../base/sdk/entity.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/c_csplayerresource.h"

constexpr float bar_width = 2.f;
constexpr float bar_height = 2.f;
constexpr float max_distance = 450.f;

namespace esp_renderer
{
  void esp_strings( esp_box_t& box, esp_objects_t& object, int max_bar_width, float& offset )
  {
    if( object.string == "" )
      return;

    auto get_current_font = [ & ]( int idx )
    {
      switch( idx )
      {
      case font_default:
        return g_fonts.esp;
        break;
      case font_pixel:
        return g_fonts.pixel;
        break;
      case font_icon:
        return g_fonts.weapon_icons;
        break;
      }
    };

    ImGui::PushFont( get_current_font( object.font_type ) );
    auto text_size = ImGui::CalcTextSize( object.string.c_str( ) );
    ImGui::PopFont( );

    float add = ( ( bar_width * 2.f ) + 1.f ) * max_bar_width;
    float add_h = ( ( bar_height * 2.f ) + 1.f ) * max_bar_width;

    auto render_string = [ & ]( float x, float y, bool center = false )
    {
      auto fl = object.dropshadow ? dropshadow_light : outline_light;

      g_render->string( x, y, object.color.new_alpha( object.color.a( ) * object.custom_alpha ), center ? centered_x | fl : fl, get_current_font( object.font_type ), object.string.c_str( ) );
    };

    switch( object.pos_type )
    {
    case esp_left:
      render_string( box.x - text_size.x - add - 3.f, box.y + offset );
      break;
    case esp_up:
      render_string( box.x + box.w / 2.f, box.y - add_h - text_size.y - offset - 3.f, true );
      break;
    case esp_right:
      render_string( box.x + box.w + add + 4.f, box.y + offset );
      break;
    case esp_down:
      render_string( box.x + box.w / 2.f, box.y + box.h + add_h + offset + 3.f, true );
      break;
    }

    offset += ( text_size.y );
  }

  void esp_bars( esp_box_t& box, esp_objects_t& object, int& offset )
  {
    if( object.bar <= 0.f )
      return;

    float bar_value = std::clamp( object.bar, 0.f, object.bar_max );

    const auto outline_color = color( 50, 50, 50, object.color.a( ) * object.custom_alpha );
    auto bar_color = object.color.new_alpha( object.color.a( ) * object.custom_alpha );

    auto text_color = color( 255, 255, 255, bar_color.a( ) * object.custom_alpha );

    float add = ( ( bar_width * 2.f ) + 1.f ) * offset;
    float add_h = ( ( bar_height * 2.f ) + 1.f ) * offset;
    float bar_h = std::clamp( ( box.h - ( ( box.h * bar_value ) / object.bar_max ) ), 0.f, box.h );
    float bar_w = std::clamp( ( ( box.w * bar_value ) / object.bar_max ), 0.f, box.w );

    auto value = std::to_string( ( int )object.bar );

    switch( object.pos_type )
    {
    case esp_left:
    {
      g_render->filled_rect( ( box.x - ( bar_width * 2.f ) ) - 2.f - add, box.y - 1.f, bar_width * 2.f, box.h + 2.f, outline_color );
      g_render->filled_rect( ( box.x - ( bar_width * 2.f ) ) - 1.f - add, box.y + bar_h, bar_width, box.h - bar_h, bar_color );

      if( object.bar < object.bar_max )
        g_render->string( ( box.x - ( bar_width * 2.f ) ) - add + 1.f, ( box.y + bar_h ) - 3.f, text_color, centered_x | outline_, g_fonts.pixel, value.c_str( ) );
    }
    break;
    case esp_up:
    {
      g_render->filled_rect( box.x - 1.f, box.y - ( bar_height * 2.f ) - 2.f - add_h, box.w + 2.f, bar_height * 2.f, outline_color );
      g_render->filled_rect( box.x, box.y - ( bar_height * 2.f ) - 1.f - add_h, bar_w, bar_height, bar_color );

      if( object.bar < object.bar_max )
        g_render->string( box.x + bar_w, box.y - ( bar_height * 2.f ) - add_h, text_color, centered_x | centered_y | outline_, g_fonts.pixel, value.c_str( ) );
    }
    break;
    case esp_right:
    {
      g_render->filled_rect( box.x + box.w + add + 2.f, box.y - 1.f, bar_width * 2.f, box.h + 2.f, outline_color );
      g_render->filled_rect( box.x + box.w + add + 3.f, box.y + bar_h, bar_width, box.h - bar_h, bar_color );

      if( object.bar < object.bar_max )
        g_render->string( box.x + box.w + add + 4.f, ( box.y + bar_h ) - 1.f, text_color, centered_x | outline_, g_fonts.pixel, value.c_str( ) );
    }
    break;
    case esp_down:
    {
      g_render->filled_rect( box.x - 1.f, box.y + box.h + add_h + 2.f, box.w + 2.f, bar_height * 2.f, outline_color );
      g_render->filled_rect( box.x, box.y + box.h + add_h + 3.f, bar_w, bar_height, bar_color );

      if( object.bar < object.bar_max )
        g_render->string( box.x + bar_w, box.y + box.h + add_h + 5.f, text_color, centered_x | centered_y | outline_, g_fonts.pixel, value.c_str( ) );
    }
    break;
    }

    offset++;
  }
}

std::string get_esp_projectile_name( model_t* model, const int& class_id )
{
  switch( class_id )
  {
  case( int )CBaseCSGrenadeProjectile:
    return strstr( model->name, xor_str( "flashbang" ).c_str( ) ) ? xor_str( "FLASH" ) : xor_str( "HE GRENADE" );
    break;
  case( int )CDecoyProjectile:
    return xor_str( "DECOY" );
    break;
  case( int )CMolotovProjectile:
  case( int )CIncendiaryGrenade:
    return xor_str( "FIRE" );
    break;
  case( int )CSensorGrenadeProjectile:
    return xor_str( "SENSOR" );
    break;
  case( int )CSmokeGrenadeProjectile:
    return xor_str( "SMOKE" );
    break;
  }
  return xor_str( " " );
}

const char8_t* get_esp_projectile_icon( model_t* model, const int& class_id )
{
  switch( class_id )
  {
  case( int )CBaseCSGrenadeProjectile:
    return strstr( model->name, xor_str( "flashbang" ).c_str( ) ) ? u8"\uE02B" : u8"\uE02C";
    break;
  case( int )CDecoyProjectile:
    return u8"\uE02F";
    break;
  case( int )CMolotovProjectile:
  case( int )CIncendiaryGrenade:
    return u8"\uE02E";
    break;
  case( int )CSmokeGrenadeProjectile:
    return u8"\uE02D";
    break;
  }
  return u8" ";
}

bool is_esp_projectile( int class_id )
{
  switch( class_id )
  {
  case( int )CBaseCSGrenadeProjectile:
  case( int )CDecoyProjectile:
  case( int )CMolotovProjectile:
  case( int )CSensorGrenadeProjectile:
  case( int )CSmokeGrenadeProjectile:
  case( int )CInferno:
    return true;
    break;
  }
  return false;
}

bool is_fake_ducking( c_csplayer* player, c_esp_store::flags_info_t& flags )
{
  auto state = player->animstate( );
  if( !state )
    return false;

  if( state->anim_duck_amount && player->flags( ) & fl_onground && state->on_ground && !state->landing )
  {
    if( state->anim_duck_amount < 0.9f && state->anim_duck_amount > 0.5f )
    {
      if( flags.fakeducktickcount != interfaces::global_vars->tick_count )
      {
        flags.fakeducktick++;
        flags.fakeducktickcount = interfaces::global_vars->tick_count;
      }

      return flags.fakeducktick >= 16;
    }
    else
      flags.fakeducktick = 0;

    return false;
  }

  return false;
}

esp_box_t get_bounding_box( c_baseentity* entity )
{
  esp_box_t out{ };

  if( entity->is_player( ) )
  {
    auto player = ( c_csplayer* )entity;
    vector3d top, down;
    vector2d s [ 2 ];

    vector3d adjust = vector3d( 0, 0, -16 ) * player->duck_amount( );

    auto abs_origin = player->get_abs_origin( );
    down = abs_origin - vector3d( 0, 0, 1 );
    top = down + vector3d( 0, 0, 72 ) + adjust;

    out.offscreen = true;
    if( g_render->world_to_screen( top, s [ 1 ] ) && g_render->world_to_screen( down, s [ 0 ] ) )
    {
      out.offscreen = false;
      vector2d delta = s [ 1 ] - s [ 0 ];

      out.h = fabsf( delta.y );
      out.w = out.h / 2.0f;

      out.x = s [ 1 ].x - ( out.w / 2 );
      out.y = s [ 1 ].y;
    }

    return out;
  }

  vector3d min = entity->bb_mins( );
  vector3d max = entity->bb_maxs( );

  vector3d points [ 8 ] = { vector3d( min.x, min.y, min.z ), vector3d( min.x, max.y, min.z ), vector3d( max.x, max.y, min.z ), vector3d( max.x, min.y, min.z ), vector3d( max.x, max.y, max.z ), vector3d( min.x, max.y, max.z ),
    vector3d( min.x, min.y, max.z ), vector3d( max.x, min.y, max.z ) };

  int valid_bounds = 0;
  vector2d points_to_screen [ 8 ] = { };
  for( int i = 0; i < 8; i++ )
  {
    if( !g_render->world_to_screen( math::get_vector_transform( points [ i ], entity->coordinate_frame( ) ), points_to_screen [ i ] ) )
      continue;

    valid_bounds++;
  }

  out.offscreen = valid_bounds == 0;
  if( out.offscreen )
    return out;

  float left = points_to_screen [ 3 ].x;
  float top = points_to_screen [ 3 ].y;
  float right = points_to_screen [ 3 ].x;
  float bottom = points_to_screen [ 3 ].y;

  for( auto i = 1; i < 8; i++ )
  {
    if( left > points_to_screen [ i ].x )
      left = points_to_screen [ i ].x;
    if( top < points_to_screen [ i ].y )
      top = points_to_screen [ i ].y;
    if( right < points_to_screen [ i ].x )
      right = points_to_screen [ i ].x;
    if( bottom > points_to_screen [ i ].y )
      bottom = points_to_screen [ i ].y;
  }

  out.x = left;
  out.y = bottom;
  out.w = right - left;
  out.h = top - bottom;
  return out;
}

bool c_esp_store::is_valid_sound( snd_info_t& sound )
{
  for( auto i = 0; i < old_vecsound.count( ); i++ )
    if( old_vecsound [ i ].guid == sound.guid )
      return false;

  return true;
}

void c_esp_store::store_dormant( )
{
  vecsound.remove_all( );
  interfaces::engine_sound->get_active_sounds( vecsound );

  if( !vecsound.count( ) )
    return;

  for( auto& s : vecsound )
  {
    if( s.sound_source < 1 || s.sound_source > 64 )
      continue;

    if( !s.origin->valid( ) )
      continue;

    if( !this->is_valid_sound( s ) )
      continue;

    auto& sound = this->sounds [ s.sound_source ];

    auto player = ( c_csplayer* )interfaces::entity_list->get_entity( s.sound_source );
    if( !player || !player->is_alive( ) || !player->is_player( ) )
      continue;

    if( g_ctx.local->is_alive( ) )
    {
      if( player == g_ctx.local || player->team( ) == g_ctx.local->team( ) )
        continue;
    }
    else
    {
      auto spectator = g_ctx.local->get_observer_target( );
      if( spectator )
      {
        if( player == spectator || player->team( ) == g_ctx.local->team( ) )
          continue;
      }
      else
        continue;
    }

    c_game_trace tr{ };

    c_trace_filter filter{ };
    filter.skip = player;

    auto soruce = *s.origin + vector3d( 0.f, 0.f, 1.f );
    auto dest = soruce - vector3d( 0.f, 0.f, 100.f );

    interfaces::engine_trace->trace_ray( ray_t( soruce, dest ), mask_playersolid, &filter, &tr );

    if( tr.all_solid )
      sound.spot_time = -1.f;

    *s.origin = tr.fraction <= 0.97f ? tr.end : *s.origin;

    sound.flags = player->flags( );
    sound.flags |= ( tr.fraction < 0.50f ? fl_ducking : 0 ) | ( tr.fraction < 1.0f ? fl_onground : 0 );
    sound.flags &= ( tr.fraction >= 0.50f ? ~fl_ducking : 0 ) | ( tr.fraction >= 1.0f ? ~fl_onground : 0 );

    sound.update( s );
  }

  old_vecsound = vecsound;
}

bool c_esp_store::in_dormant( c_csplayer* player )
{
  auto i = player->index( );
  auto& sound_player = sounds [ i ];

  auto info = this->get_player_info( i );

  auto expired = false;

  if( std::fabs( interfaces::global_vars->real_time - sound_player.spot_time ) > 10.f )
    expired = true;

  player->target_spotted( ) = true;
  player->flags( ) = sound_player.flags;
  player->set_abs_origin( sound_player.pos );

  return !expired;
}

void c_esp_store::store_players( )
{
  const std::unique_lock< std::mutex > lock( mutexes::players );

  this->store_dormant( );

  auto& entities = g_listener_entity->get_entity( ent_player );
  if( entities.empty( ) )
    return;

  auto radar_base = func_ptrs::find_hud_element( *patterns::get_hud_ptr.as< uintptr_t** >( ), xor_c( "CCSGO_HudRadar" ) );
  auto hud_radar = ( c_hud_radar* )( radar_base - 0x14 );

  for( auto& entity : entities )
  {
    auto player = ( c_csplayer* )entity.m_entity;
    if( !player )
      continue;

    auto& info = playerinfo [ player->index( ) ];
    if( info.ptr != player )
    {
      info.reset( );
      info.ptr = player;

      sounds [ player->index( ) ].reset( );
      continue;
    }

    auto backup_flags = player->flags( );
    auto backup_origin = player->get_abs_origin( );

    if( ( player->observer_mode( ) == 4 || player->observer_mode( ) == 5 ) )
    {
      auto target = player->get_observer_target( );
      if( !target || !target->is_alive( ) )
      {
        info.valid = false;
        continue;
      }

      if( target == g_ctx.local || target->team( ) == g_ctx.local->team( ) )
        continue;

      auto& sound = sounds [ target->index( ) ];
      sound.spot_pos = player->origin( );

      if( ( sound.spot_pos - sound.pos ).length( false ) < 512.f )
      {
        sound.pos = player->origin( );
        sound.flags = player->flags( );
      }

      sound.spot_time = interfaces::global_vars->real_time;
    }

    if( !player->is_alive( ) )
    {
      info.valid = false;
      continue;
    }

    if( g_ctx.local->is_alive( ) )
    {
      if( player == g_ctx.local || player->team( ) == g_ctx.local->team( ) )
      {
        info.valid = false;
        continue;
      }
    }
    else
    {
      auto spectator = g_ctx.local->get_observer_target( );
      if( spectator && player == spectator || player->team( ) == g_ctx.local->team( ) )
      {
        info.valid = false;
        continue;
      }
    }

    if( player->dormant( ) )
    {
      info.dormant = this->in_dormant( player );
      info.dormant_origin = player->get_abs_origin( );

      if( info.dormant )
        info.dormant_alpha = std::lerp( info.dormant_alpha, 0.35f, interfaces::global_vars->frame_time * 2.5f );
      else
        info.dormant_alpha = std::lerp( info.dormant_alpha, 0.f, interfaces::global_vars->frame_time * 4.f );

      if( info.dormant_alpha <= 0.f )
      {
        info.valid = false;

        player->flags( ) = backup_flags;
        player->set_abs_origin( backup_origin );
        continue;
      }
    }
    else
    {
      info.dormant = false;
      info.dormant_alpha = std::lerp( info.dormant_alpha, 1.f, interfaces::global_vars->frame_time * 3.f );

      info.dormant_hp = player->health( );
      info.name = player->get_name( );

      info.undormant_flags = player->flags( );
      info.undormant_origin = player->get_abs_origin( );

      auto& sound = this->sounds [ player->index( ) ];
      sound.pos = info.undormant_origin;
      sound.flags = info.undormant_flags;
      sound.spot_time = interfaces::global_vars->real_time;
    }

    if( radar_base && hud_radar && player->dormant( ) && player->team( ) != g_ctx.local->team( ) && player->target_spotted( ) )
    {
      info.dormant_hp = hud_radar->radar_info [ player->index( ) ].health;
      info.name = player->get_name( hud_radar->radar_info [ player->index( ) ].name );
    }

    if( !info.dormant_hp )
    {
      if( player->dormant( ) )
      {
        info.valid = false;

        player->flags( ) = backup_flags;
        player->set_abs_origin( backup_origin );
      }

      continue;
    }

    info.valid = true;

    info.box = get_bounding_box( player );

    info.origin = player->get_abs_origin( );
    info.index = player->index( );
    info.health = std::clamp( info.dormant_hp, 0, 100 );

    auto& flags = info.flags;
    flags.hashelmet = player->has_helmet( );
    flags.armorvalue = player->armor_value( );

    if( !info.dormant )
    {
      flags.zoom = player->is_scoped( );
      flags.fakeduck = is_fake_ducking( player, flags );
      flags.defusing = player->is_defusing( );
      flags.havekit = player->has_defuser( );
      // flags.havebomb = interfaces::player_resource->player_c4() == player->index();

      auto record = g_animation_fix->get_latest_record( player );
      flags.deffensive = !record;

      auto layer = &player->anim_overlay( ) [ 1 ];
      if( layer->owner )
      {
        auto sequence = player->get_sequence_activity( layer->sequence );
        flags.reloading = sequence == 967 && layer->weight != 0.f;
      }

      flags.aimtarget = g_rage_bot->target && g_rage_bot->target == player;

      auto weapon = player->get_active_weapon( );
      if( weapon )
      {
        info.weaponicon = ( const char* )weapon->get_weapon_icon( );
        info.weaponname = weapon->get_weapon_name( );
        info.miscweapon = weapon->is_misc_weapon( );

        auto weapon_info = weapon->get_weapon_info( );
        if( weapon_info )
        {
          info.ammo = weapon->clip1( );
          info.maxammo = weapon_info->max_ammo_1;
        }
      }

      auto hdr = interfaces::model_info->get_studio_model( player->get_model( ) );
      if( !hdr )
        continue;

      auto cache = player->bone_cache( ).base( );

      for( int j = 0; j < hdr->bones; j++ )
      {
        auto bone = hdr->get_bone( j );

        if( bone && ( bone->flags & 0x100 ) && ( bone->parent != -1 ) )
        {
          g_render->world_to_screen( cache [ j ].get_origin( ), info.bone_pos_child [ j ], true );
          g_render->world_to_screen( cache [ bone->parent ].get_origin( ), info.bone_pos_parent [ j ], true );
        }
      }
    }
  }
}

void c_esp_store::store_weapons( )
{
  const std::unique_lock< std::mutex > lock( mutexes::weapons );

  auto& weapon_array = g_listener_entity->get_entity( ent_weapon );
  if( weapon_array.empty( ) )
    return;

  for( auto& weapon : weapon_array )
  {
    auto entity = ( c_basecombatweapon* )weapon.m_entity;
    if( !entity )
      continue;

    auto get_esp_alpha = [ & ]( vector3d origin )
    {
      float distance = origin.dist_to( entity->get_abs_origin( ) );
      return 255.f - std::clamp( ( 255.f * distance ) / max_distance, 0.f, 255.f );
    };

    auto& info = weaponinfo [ entity->index( ) ];

    auto client_class = entity->get_client_class( );
    if( !client_class )
    {
      if( info.valid )
        info.reset( );
      continue;
    }

    info.class_id = client_class->class_id;

    if( g_ctx.local && g_ctx.local->is_alive( ) )
      info.alpha = get_esp_alpha( g_ctx.local->get_abs_origin( ) );
    else
    {
      auto spectator = g_ctx.local->get_observer_target( );
      if( !spectator )
      {
        if( info.valid )
          info.reset( );
        continue;
      }

      info.alpha = get_esp_alpha( spectator->get_abs_origin( ) );
    }

    info.box = get_bounding_box( entity );
    info.valid = !info.box.offscreen && info.alpha > 0.f && client_class->class_id != CPlantedC4;
    info.proj = is_esp_projectile( client_class->class_id );

    if( info.proj )
    {
      info.alpha = 255.f;
      info.valid = !info.box.offscreen && client_class->class_id != CPlantedC4;

      if( info.class_id == CInferno )
      {
        info.expire_inferno = ( ( ( *( float* )( ( uintptr_t )entity + 0x20 ) ) + 7.03125f ) - interfaces::global_vars->cur_time );

        bool* m_bFireIsBurning = entity->fire_is_burning( );
        int* m_fireXDelta = entity->fire_x_delta( );
        int* m_fireYDelta = entity->fire_y_delta( );
        int* m_fireZDelta = entity->fire_z_delta( );
        int m_fireCount = entity->fire_count( );

        vector3d average_vector = vector3d( 0, 0, 0 );
        for( int i = 0; i <= m_fireCount; i++ )
        {
          if( !m_bFireIsBurning [ i ] )
            continue;

          vector3d fire_origin = vector3d( m_fireXDelta [ i ], m_fireYDelta [ i ], m_fireZDelta [ i ] );
          float delta = fire_origin.length( true ) + 14.4f;
          if( delta > info.inferno_range )
            info.inferno_range = delta;

          average_vector += fire_origin;
        }

        if( m_fireCount <= 1 )
          info.origin = entity->get_abs_origin( );
        else
          info.origin = ( average_vector / m_fireCount ) + entity->get_abs_origin( );
      }
      else
        info.origin = entity->get_abs_origin( );

      info.did_smoke = info.class_id == CSmokeGrenadeProjectile && entity->did_smoke_effect( );
      if( info.did_smoke )
      {
        float tick_time = ( entity->smoke_effect_tick_begin( ) * interfaces::global_vars->interval_per_tick ) + 17.5f;
        info.expire_smoke = tick_time - interfaces::global_vars->cur_time;
      }

      if( entity->nade_exploded( ) && !info.did_smoke && info.class_id != CInferno )
      {
        if( info.valid )
          info.reset( );
        continue;
      }

      info.name = get_esp_projectile_name( entity->get_model( ), client_class->class_id );
      info.icon = ( const char* )get_esp_projectile_icon( entity->get_model( ), client_class->class_id );
    }
    else
    {
      int owner = entity->owner( );
      if( info.alpha <= 0.f || owner > 0 || client_class->class_id == CPlantedC4 )
      {
        if( info.valid )
          info.reset( );
        continue;
      }

      auto wpn_info = entity->get_weapon_info( );
      if( !wpn_info )
      {
        if( info.valid )
          info.reset( );
        continue;
      }

      info.ammo = entity->clip1( );
      info.ammo_max = wpn_info->max_ammo_1;
      info.name = entity->get_weapon_name( );
      info.icon = ( const char* )entity->get_weapon_icon( );
    }
  }
}

void c_esp_store::on_paint_traverse( )
{
  if( !g_ctx.local || !g_ctx.in_game )
    return;

  if( g_ctx.round_start )
    return;

  g_menu->store_bomb( );

  this->store_players( );
  this->store_weapons( );
}

void c_esp_store::on_changed_map( )
{
  this->reset_all( );
}

void c_esp_store::on_game_events( c_game_event* event )
{
  if( std::strcmp( event->get_name( ), xor_c( "round_start" ) ) )
    return;

  this->reset_all( );
}
