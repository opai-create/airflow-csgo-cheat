#pragma once
#include "../../base/sdk/entity.h"
#include "../../base/global_context.h"

#include <mutex>
#include <array>

enum esp_fonts_t
{
  font_default,
  font_pixel,
  font_icon
};

enum esp_objs_t
{
  esp_bar,
  esp_string,
  esp_icon
};

enum esp_poses_t
{
  esp_left,
  esp_up,
  esp_right,
  esp_down
};

const int max_objs_count = 20;

struct esp_box_t
{
  bool offscreen{ };

  float x{ };
  float y{ };
  float w{ };
  float h{ };

  __forceinline void reset( )
  {
    offscreen = true;

    x = 0.f;
    y = 0.f;
    w = 0.f;
    h = 0.f;
  }
};

struct esp_objects_t
{
  bool filled{ };

  int pos_type{ };

  color color{ };

  float bar{ };
  float bar_max{ };

  std::string string{ };

  int font_type{ };
  bool dropshadow{ };

  float custom_alpha = 1.f;

  __forceinline void reset( )
  {
    if( filled )
    {
      pos_type = 0;

      color.set( 0, 0, 0, 0 );

      bar = 0.f;
      bar_max = 0.f;
      custom_alpha = 1.f;

      string = "";
      font_type = 0;

      filled = false;
      dropshadow = false;
    }
  }
};

namespace esp_renderer
{
  extern void esp_strings( esp_box_t& box, esp_objects_t& object, int max_bar_width, float& offset );
  extern void esp_bars( esp_box_t& box, esp_objects_t& object, int& offset );
}

class c_esp_store
{
private:
  __forceinline void reset_all( )
  {
    for( auto& i : weaponinfo )
      i.reset( );

    for( auto& i : playerinfo )
      i.reset( true );

    old_vecsound.remove_all( );
    vecsound.remove_all( );
  }

  bool is_valid_sound( snd_info_t& sound );
  void store_dormant( );
  bool in_dormant( c_csplayer* player );
  void store_players( );
  void store_weapons( );

public:
  struct sound_info_t
  {
    int flags{ };
    float spot_time{ };
    vector3d pos{ };
    vector3d spot_pos{ };

    __forceinline void update( snd_info_t& sound )
    {
      spot_time = interfaces::global_vars->real_time;
      pos = *sound.origin;
    }

    __forceinline void reset( )
    {
      spot_time = 0.f;
      pos.reset( );
      spot_pos.reset( );
      flags = 0;
    }
  };

  struct flags_info_t
  {
    bool hashelmet{ };
    bool zoom{ };
    bool aimtarget{ };
    bool fakeduck{ };
    bool deffensive{ };
    bool reloading{ };
    bool havebomb{ };
    bool havekit{ };
    bool defusing{ };

    int armorvalue{ };
    int fakeducktick{ };
    int fakeducktickcount{ };

    __forceinline void reset( )
    {
      hashelmet = false;
      zoom = false;
      aimtarget = false;
      fakeduck = false;
      deffensive = false;
      reloading = false;
      havebomb = false;
      havekit = false;
      defusing = false;

      armorvalue = 0;
      fakeducktick = 0;
      fakeducktickcount = 0;
    }
  };

  struct player_info_t
  {
    bool valid{ };
    bool dormant{ };
    bool miscweapon{ };

    float ammo{ };
    float maxammo{ };
    float health{ };
    float dormant_alpha{ };

    int index{ };
    int dormant_hp{ };
    int undormant_flags{ };

    std::string name{ };
    std::string weaponicon{ };
    std::string weaponname{ };

    c_csplayer* ptr{ };

    esp_box_t box{ };
    flags_info_t flags{ };

    vector3d origin{ };
    vector3d dormant_origin{ };
    vector3d undormant_origin{ };

    color dormantclr{ };

    vector2d bone_pos_child [ 128 ];
    vector2d bone_pos_parent [ 128 ];

    std::array< esp_objects_t, max_objs_count > objs{ };

    void reset( bool erase_player = false )
    {
      if( erase_player )
        ptr = nullptr;

      valid = false;
      dormant = false;
      miscweapon = false;

      ammo = 0.f;
      maxammo = 0.f;
      health = 0.f;
      dormant_alpha = 0.f;

      index = 0;
      dormant_hp = 0;
      undormant_flags = 0;

      name = "";
      weaponicon = "";
      weaponname = "";

      origin.reset( );
      dormant_origin.reset( );
      undormant_origin.reset( );

      box.reset( );
      flags.reset( );

      dormantclr = { };

      for( int i = 0; i < max_objs_count; ++i )
        objs [ i ].reset( );

      std::memset( bone_pos_child, 0, sizeof( bone_pos_child ) );
      std::memset( bone_pos_parent, 0, sizeof( bone_pos_parent ) );
    }
  };

  std::array< player_info_t, 65 > playerinfo{ };

  __forceinline void reset_player_info( int idx )
  {
    playerinfo [ idx ].reset( );
  }

  __forceinline player_info_t* get_player_info( int idx )
  {
    return &playerinfo [ idx ];
  }

  struct world_info_t
  {
    bool valid{ };
    bool did_smoke{ };
    bool proj{ };

    int class_id{ };

    float inferno_range{ };

    float expire_inferno{ };
    float expire_smoke{ };

    float ease_inferno{ };
    float ease_smoke{ };

    float ammo{ };
    float ammo_max{ };

    float alpha{ };

    std::string name{ };
    std::string icon{ };

    vector3d origin{ };

    esp_box_t box{ };

    std::array< esp_objects_t, max_objs_count > objs{ };

    __forceinline void reset( )
    {
      valid = false;
      did_smoke = false;
      proj = false;

      class_id = 0;

      inferno_range = 0.f;

      expire_inferno = 0.f;
      expire_smoke = 0.f;

      ease_inferno = 0.f;
      ease_smoke = 0.f;

      ammo = 0.f;
      ammo_max = 0.f;

      alpha = 0.f;

      name = "";
      icon = "";

      origin.reset( );
      box.reset( );

      for( int i = 0; i < max_objs_count; ++i )
        objs [ i ].reset( );
    }
  };

  std::array< world_info_t, 4096 > weaponinfo{ };

  __forceinline void reset_weapon_info( int idx )
  {
    weaponinfo [ idx ].reset( );
  }

  __forceinline world_info_t* get_weapon_info( int idx )
  {
    return &weaponinfo [ idx ];
  }

  std::array< sound_info_t, 65 > sounds{ };
  c_utlvector< snd_info_t > old_vecsound{ };
  c_utlvector< snd_info_t > vecsound{ };

  void on_paint_traverse( );
  void on_changed_map( );
  void on_game_events( c_game_event* event );
};