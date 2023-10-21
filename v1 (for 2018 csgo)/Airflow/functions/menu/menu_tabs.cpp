#include "menu.h"
#include "ui_structs.h"

#include "../config_system.h"
#include "../config_vars.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"
#include "../../base/tools/math.h"
#include "../../base/other/byte_arrays.h"

#include "../skins/skins.h"

#include <ShlObj.h>
#include <algorithm>
#include <shellapi.h>
#include <Windows.h>
#include <map>

std::vector< std::string > visual_tabs = {
  xor_str( "Enemy" ),
  xor_str( "Local" ),
  xor_str( "Weapons" ),
  xor_str( "Modulate" ),
  xor_str( "Effects" ),
  xor_str( "Camera" ),
};

std::vector< std::string > misc_tabs = {
  xor_str( "Movement" ),
  xor_str( "Events" ),
  xor_str( "Other" ),
};

#define begin_child( text )                                                                                                                                                                                                          \
  ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f, this->get_alpha( ) ) );                                                                                                                                               \
  ImGui::Text( ( text ) );                                                                                                                                                                                                           \
  ImGui::PopStyleColor( );                                                                                                                                                                                                           \
  ImGui::ItemSize( ImVec2( 0, 1 ) );                                                                                                                                                                                                 \
  ImGui::BeginGroup( );

#define end_child                                                                                                                                                                                                                    \
  ImGui::EndGroup( );                                                                                                                                                                                                                \
  ImGui::ItemSize( ImVec2( 0, 9 ) );

int selector = 0;
int selector_visuals = 0;

std::vector< std::string > config_list{ };
std::vector< std::string > empty_list = { xor_str( "Config folder is empty!" ) };

bool update_configs = false;

typedef void ( *LPSEARCHFUNC )( LPCTSTR lpszFileName );

BOOL search_files( LPCTSTR lpszFileName, LPSEARCHFUNC lpSearchFunc, BOOL bInnerFolders )
{
  LPTSTR part;
  char tmp [ MAX_PATH ];
  char name [ MAX_PATH ];

  HANDLE hSearch = NULL;
  WIN32_FIND_DATA wfd;
  memset( &wfd, 0, sizeof( WIN32_FIND_DATA ) );

  if( bInnerFolders )
  {
    if( GetFullPathNameA( lpszFileName, MAX_PATH, tmp, &part ) == 0 )
      return FALSE;
    strcpy( name, part );
    strcpy( part, xor_c( "*.*" ) );
    wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    if( !( ( hSearch = FindFirstFileA( tmp, &wfd ) ) == INVALID_HANDLE_VALUE ) )
      do
      {
        if( !strncmp( wfd.cFileName, xor_c( "." ), 1 ) || !strncmp( wfd.cFileName, xor_c( ".." ), 2 ) )
          continue;

        if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
          char next [ MAX_PATH ];
          if( GetFullPathNameA( lpszFileName, MAX_PATH, next, &part ) == 0 )
            return FALSE;
          strcpy( part, wfd.cFileName );
          strcat( next, xor_c( "\\" ) );
          strcat( next, name );

          search_files( next, lpSearchFunc, TRUE );
        }
      } while( FindNextFileA( hSearch, &wfd ) );
    FindClose( hSearch );
  }

  if( ( hSearch = FindFirstFileA( lpszFileName, &wfd ) ) == INVALID_HANDLE_VALUE )
    return TRUE;
  do
    if( !( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
    {
      char file [ MAX_PATH ];
      if( GetFullPathNameA( lpszFileName, MAX_PATH, file, &part ) == 0 )
        return FALSE;
      strcpy( part, wfd.cFileName );

      lpSearchFunc( wfd.cFileName );
    }
  while( FindNextFileA( hSearch, &wfd ) );
  FindClose( hSearch );
  return TRUE;
}

void read_configs( LPCTSTR lpszFileName )
{
  config_list.push_back( lpszFileName );
}

void refresh_configs( )
{
  static TCHAR path [ MAX_PATH ];
  std::string folder, file;

  if( SUCCEEDED( SHGetFolderPathA( NULL, CSIDL_PERSONAL, NULL, 0, path ) ) )
  {
    config_list.clear( );
    std::string config_dir = std::string( path ) + xor_c( "\\airflow\\*" );
    search_files( config_dir.c_str( ), read_configs, FALSE );
  }
}

const char* rage_hitboxes [] = {
  xor_strs::hitbox_head.c_str( ),
  xor_strs::hitbox_body.c_str( ),
  xor_strs::hitbox_limbs.c_str( ),
};

const char* weapon_configs [] = {
  xor_strs::weapon_default.c_str( ),
  xor_strs::weapon_auto.c_str( ),
  xor_strs::weapon_heavy_pistols.c_str( ),
  xor_strs::weapon_pistols.c_str( ),
  xor_strs::weapon_ssg08.c_str( ),
  xor_strs::weapon_awp.c_str( ),
};

const char* aa_cond [] = {
  xor_strs::aa_stand.c_str( ),
  xor_strs::aa_move.c_str( ),
  xor_strs::aa_air.c_str( ),
};

const char* aa_pitch [] = {
  xor_strs::aa_disabled.c_str( ),
  xor_strs::aa_pitch_down.c_str( ),
  xor_strs::aa_pitch_up.c_str( ),
  xor_strs::aa_pitch_minimal.c_str( ),
};

const char* aa_yaw [] = {
  xor_strs::aa_disabled.c_str( ),
  xor_strs::aa_yaw_back.c_str( ),
  xor_strs::aa_yaw_spin.c_str( ),
  xor_strs::aa_yaw_crooked.c_str( ),
};

const char* aa_jitter_yaw [] = {
  xor_strs::aa_disabled.c_str( ),
  xor_strs::aa_jitter_center.c_str( ),
  xor_strs::aa_jitter_offset.c_str( ),
  xor_strs::aa_jitter_random.c_str( ),
};

const char* aa_desync_type [] = {
  xor_strs::aa_disabled.c_str( ),
  xor_strs::aa_default.c_str( ),
  xor_strs::aa_desync_jitter.c_str( ),
};

const char* vis_chams_type [] = {
  xor_strs::vis_chams_textured.c_str( ),
  xor_strs::vis_chams_metallic.c_str( ),
  xor_strs::vis_chams_flat.c_str( ),
  xor_strs::vis_chams_glass.c_str( ),
  xor_strs::vis_chams_glow.c_str( ),
  xor_strs::vis_chams_bubble.c_str( ),
  xor_strs::vis_chams_money.c_str( ),
  xor_strs::vis_chams_fadeup.c_str( ),
};

const char* buybot_main []{
  xor_strs::buybot_none.c_str( ),
  xor_strs::weapon_auto.c_str( ),
  xor_strs::weapon_ssg08.c_str( ),
  xor_strs::weapon_awp.c_str( ),
  xor_strs::weapon_negev.c_str( ),
  xor_strs::weapon_m249.c_str( ),
  xor_strs::weapon_ak47.c_str( ),
  xor_strs::weapon_aug.c_str( ),
};

const char* buybot_second []{
  xor_strs::buybot_none.c_str( ),
  xor_strs::weapon_duals.c_str( ),
  xor_strs::weapon_p250.c_str( ),
  xor_strs::weapon_cz.c_str( ),
  xor_strs::weapon_heavy_pistols.c_str( ),
};

const char* enemy_models []{
  xor_strs::chams_visible.c_str( ),
  xor_strs::chams_xqz.c_str( ),
  xor_strs::chams_history.c_str( ),
  xor_strs::chams_onshot.c_str( ),
  xor_strs::chams_ragdolls.c_str( ),
};

const char* local_models []{
  xor_strs::chams_visible.c_str( ),
  xor_strs::chams_viewmodel.c_str( ),
  xor_strs::chams_wpn.c_str( ),
  xor_strs::chams_attachments.c_str( ),
  xor_strs::chams_fake.c_str( ),
};

const char* hitsound []{
  xor_strs::aa_disabled.c_str( ),
  xor_strs::sound_metallic.c_str( ),
  xor_strs::sound_tap.c_str( ),
};

const char* boxes []{
  xor_strs::box_default.c_str( ),
  xor_strs::box_thin.c_str( ),
};

const char* ragdoll_gravities []{
  xor_strs::buybot_none.c_str( ),
  xor_strs::ragdoll_away.c_str( ),
  xor_strs::ragdoll_fly.c_str( ),
};

const char* target_selection []{
  xor_strs::target_distance.c_str( ),
  xor_strs::target_damage.c_str( ),
};

const char* glove_models []{ "Default", "Bloodhound", "Sporty", "Slick", "Leather Wrap", "Motorcycle", "Specialist", "Hydra" };

const char* glove_skins []{
  "Charred",
  "Snakebite",
  "Bronzed",
  "Leather",
  "Spruce DDPAT",
  "Lunar Weave",
  "Convoy",
  "Crimson Weave",
  "Superconductor",
  "Arid",
  "Slaughter",
  "Eclipse",
  "Spearmint",
  "Boom!",
  "Cool Mint",
  "Forest DDPAT",
  "Crimson Kimono",
  "Emerald Web",
  "Foundation",
  "Badlands",
  "Pandora's Box",
  "Hedge Maze",
  "Guerrilla",
  "Diamondback",
  "King Snake",
  "Imperial Plaid",
  "Overtake",
  "Racing Green",
  "Amphibious",
  "Bronze Morph",
  "Omega",
  "Vice",
  "POW!",
  "Turtle",
  "Transport",
  "Polygon",
  "Cobalt Skulls",
  "Overprint",
  "Duct Tape",
  "Arboreal",
  "Emerald",
  "Mangrove",
  "Rattler",
  "Case Hardened",
  "Crimson Web",
  "Buckshot",
  "Fade",
  "Mogul",
};

const char* masks []{
  "None",
  "Battlemask",
  "Hoxton",
  "Doll",
  "Skull",
  "Samurai",
  "Evil Clown",
  "Wolf",
  "Sheep",
  "Bunyy Gold",
  "Anaglyph",
  "Kabuki Doll",
  "Dallas",
  "Pumpkin",
  "Sheep Bloody",
  "Devil Plastic",
  "Boar",
  "Chains",
  "Tiki",
  "Bunny",
  "Sheep Gold",
  "Zombie Plastic",
  "Chicken",
  "Skull Gold",
  "Demoman",
  "Engineer",
  "Heavy",
  "Medic",
  "Pyro",
  "Scout",
  "Sniper",
  "Solider",
  "Spy",
  "Holiday Light",
};

const char* knife_models []{
  "Default",
  "Bayonet",
  "Flip knife",
  "Gut knife",
  "Karambit",
  "M9 Bayonet",
  "Huntsman knife",
  "Falchion knife",
  "Bowie knife",
  "Butterfly",
  "Shadow Daggers",
};

const char* skin_weapon_configs [] = {
  "Deagle",
  "Duals",
  "Five seven",
  "Glock",
  "AK 47",
  "AUG",
  "AWP",
  "Famas",
  "G3SG1",
  "Galil",
  "M249",
  "M4A1",
  "M4A1 S",
  "MAC 10",
  "P 90",
  "MP5",
  "UMP45",
  "XM1014",
  "Bizon",
  "MAG 7",
  "Negev",
  "Sawed off",
  "Tec 9",
  "P2000",
  "MP7",
  "MP9",
  "Nova",
  "P250",
  "Scar 20",
  "SG 553",
  "Scout",
  "USP S",
  "CZ 75",
  "Revolver",
  "Knife",
};

const char* agents []{
  ( "Default" ),
  ( "Danger zone A" ),
  ( "Danger zone B" ),
  ( "Danger zone C" ),
  ( "Cmdr. Davida Fernandez" ),
  ( "Cmdr. Frank Baroud" ),
  ( "Lieutenant Rex Krikey" ),
  ( "Michael Syfers" ),
  ( "Operator" ),
  ( "Special Agent Ava" ),
  ( "Markus Delrow" ),
  ( "Sous-Lieutenant Medic" ),
  ( "Chem-Haz Capitaine" ),
  ( "Chef d'Escadron Rouchard" ),
  ( "Aspirant" ),
  ( "Officer Jacques Beltram" ),
  ( "D Squadron Officer" ),
  ( "B Squadron Officer" ),
  ( "Seal Team 6 Soldier" ),
  ( "Buckshot" ),
  ( "Lt. Commander Ricksaw" ),
  ( "Buckshot 2" ),
  ( "3rd Commando Company" ),
  ( "'Two Times' McCoy" ),
  ( "'Two Times' McCoy 2" ),
  ( "Primeiro Tenente 1st Battalion" ),
  ( "Cmdr. Mae Jamison" ),
  ( "1st Lieutenant Farlow" ),
  ( "John 'Van Healen' Kask" ),
  ( "Bio-Haz Specialist" ),
  ( "Sergeant Bombson" ),
  ( "Chem-Haz Specialist" ),
  ( "Lieutenant Farlow" ),
  ( "Getaway Sally" ),
  ( "Number K" ),
  ( "Little Kev" ),
  ( "Safecracker Voltzmann" ),
  ( "Bloody Darryl The Strapped" ),
  ( "Sir Bloody Loudmouth Darryl" ),
  ( "Sir Bloody Darryl Royale" ),
  ( "Sir Bloody Skullhead Darryl" ),
  ( "Sir Bloody Silent Darryl" ),
  ( "Sir Bloody Miami Darryl" ),
  ( "Street Soldier" ),
  ( "Soldier" ),
  ( "Slingshot" ),
  ( "Enforcer" ),
  ( "Mr. Muhlik" ),
  ( "Prof. Shahmat" ),
  ( "Osiris" ),
  ( "Ground Rebel" ),
  ( "The Elite Mr. Muhlik" ),
  ( "Trapper" ),
  ( "Trapper Aggressor" ),
  ( "Vypa Sista of the Revolution" ),
  ( "Col. Mangos Dabisi" ),
  ( "'Medium Rare' Crasswater" ),
  ( "Crasswater The Forgotten" ),
  ( "Elite Trapper Solman" ),
  ( "'The Doctor' Romanov" ),
  ( "Blackwolf" ),
  ( "Maximus" ),
  ( "Dragomir" ),
  ( "Rezan The Ready" ),
  ( "Rezan the Redshirt" ),
};

const char* skyboxes []{ xor_strs::aa_default.c_str( ), "Tibet", "Bagage", "Italy", "Jungle", "Office", "Daylight 1", "Daylight 2", "Vertigo blue", "Vertigo", "Day", "Nuke blank", "Venice", "Daylight 3", "Daylight 4", "Cloudy",
  "Night", "Nightb", "Night fat", "Dust", "Vietnam", "Embassy", "Custom" };

const char* tracers []{
  xor_strs::tracer_beam.c_str( ),
  xor_strs::tracer_line.c_str( ),
};

void c_menu::draw_ui_items( )
{
  auto window_pos = this->get_window_pos( );
  auto prev_pos = ImGui::GetCursorPos( );
  ImGui::SetCursorPos( ImVec2( 215, 77 ) );

  ImRect window_bb = ImRect( window_pos + ImVec2( 160, 47 ), window_pos + ImVec2( 725, 520 ) );
  ImGui::PushClipRect( window_bb.Min, window_bb.Max, false );

  float old_alpha = this->get_alpha( );

  static int prev_sel = 0;
  if( tab_selector != prev_sel )
  {
    for( auto& a : item_animations )
    {
      a.second.reset( );
      tab_alpha = 0.f;
    }
    prev_sel = tab_selector;
  }

  // change alpha everytime user clicks on new tab
  this->create_animation( tab_alpha, g_cfg.misc.menu && tab_selector == prev_sel, 0.2f, skip_disable | lerp_animation );

  alpha = tab_alpha;

  if( tab_selector == 5 )
  {
    if( !update_configs )
    {
      refresh_configs( );
      update_configs = true;
    }
  }
  else
    update_configs = false;

  ImGui::BeginChild( xor_c( "##tab_child" ), ImVec2( ), true );
  {
    switch( tab_selector )
    {
    case 0:
    {
      ImGui::Columns( 2, 0, false );
      ImGui::SetColumnOffset( 1, 270 );
      {
        begin_child( xor_c( "Choose your weapon" ) )
        {
          this->combo( xor_c( "Setting up..." ), &g_cfg.rage.group_type, weapon_configs, IM_ARRAYSIZE( weapon_configs ) );
          if( g_cfg.rage.group_type > 0 )
          {
            auto name = xor_str( "Enable config##group_" ) + std::to_string( g_cfg.rage.group_type );
            this->checkbox( name.c_str( ), &g_cfg.rage.weapon [ g_cfg.rage.group_type ].enable );
          }
        }
        end_child;

        begin_child( xor_c( "General" ) )
        {
          this->checkbox( xor_c( "Enable" ), &g_cfg.rage.enable );
          this->checkbox( xor_c( "Auto fire" ), &g_cfg.rage.auto_fire );
          this->checkbox( xor_c( "Delay untill unlag" ), &g_cfg.rage.delay_shot );
          this->key_bind( xor_c( "Force body" ), g_cfg.binds [ force_body_b ] );
          this->key_bind( xor_c( "Override damage" ), g_cfg.binds [ override_dmg_b ] );
        }
        end_child;

        begin_child( xor_c( "Exploits" ) )
        {
          this->key_bind( xor_c( "Double tap" ), g_cfg.binds [ dt_b ] );
          this->key_bind( xor_c( "Hide shots" ), g_cfg.binds [ hs_b ] );

          this->checkbox( xor_c( "Off defensive" ), &g_cfg.rage.anti_mrx );
        }
        end_child;

        begin_child( xor_c( "Ping spike" ) )
        {
          this->key_bind( xor_c( "Ping spike" ), g_cfg.binds [ spike_b ] );
          this->slider_int( xor_c( "Amount" ), &g_cfg.rage.spike_amt, 0, 1000, xor_c( "%d%ms" ) );
          this->checkbox( xor_c( "Compensate spike" ), &g_cfg.rage.adaptive_spike );
        }
        end_child;
      }
      ImGui::NextColumn( );
      {
        auto& weapon_settings = g_cfg.rage.weapon [ g_cfg.rage.group_type ];

        begin_child( xor_c( "Hitscan" ) )
        {
          if( g_cfg.rage.group_type == global || g_cfg.rage.group_type == auto_snipers || g_cfg.rage.group_type == scout || g_cfg.rage.group_type == awp )
            this->checkbox( xor_c( "Auto scope" ), &weapon_settings.auto_scope );

          this->multi_combo( xor_c( "Hitboxes" ), weapon_settings.hitboxes, { xor_str( "Head" ), xor_str( "Chest" ), xor_str( "Stomach" ), xor_str( "Pelvis" ), xor_str( "Arms" ), xor_str( "Legs" ) } );

          this->slider_int( xor_c( "Head scale" ), &weapon_settings.scale_head, 0, 100, xor_c( "%d%%" ) );
          this->slider_int( xor_c( "Body scale" ), &weapon_settings.scale_body, 0, 100, xor_c( "%d%%" ) );

          auto dmg_str = weapon_settings.mindamage == 100 ? xor_c( "HP" ) : weapon_settings.mindamage > 100 ? xor_c( "HP + " ) + std::to_string( weapon_settings.mindamage - 100 ) : xor_c( "%dHP" );

          auto override_str = weapon_settings.damage_override == 100 ? xor_c( "HP" ) : weapon_settings.damage_override > 100 ? xor_c( "HP + " ) + std::to_string( weapon_settings.damage_override - 100 ) : xor_c( "%dHP" );

          this->slider_int( xor_c( "Hitchance" ), &weapon_settings.hitchance, 0, 100, xor_c( "%d%%" ) );
          this->slider_int( xor_c( "Min damage" ), &weapon_settings.mindamage, 1, 120, dmg_str.c_str( ) );
          this->slider_int( xor_c( "Min damage (override)" ), &weapon_settings.damage_override, 1, 120, override_str.c_str( ) );

          this->checkbox( xor_c( "Quick stop" ), &weapon_settings.quick_stop );
          this->multi_combo( xor_c( "Options##quick_stop" ), weapon_settings.quick_stop_options, { xor_c( "Early" ), xor_c( "Between shots" ), xor_c( "Force accuracy" ) } );
        }
        end_child;
      }
    }
    break;
    case 1:
    {
      ImGui::Columns( 2, 0, false );
      ImGui::SetColumnOffset( 1, 270 );
      {
        begin_child( xor_c( "Main" ) )
        {
          this->checkbox( xor_c( "Enable##antiaim" ), &g_cfg.antihit.enable );
          if( g_cfg.antihit.enable )
          {
            this->combo( xor_c( "Condition" ), &g_cfg.antihit.condition_type, aa_cond, IM_ARRAYSIZE( aa_cond ) );

            auto& cond = g_cfg.antihit.angles [ g_cfg.antihit.condition_type ];

            this->checkbox( xor_c( "At targets##antiaim" ), &cond.at_targets );

            this->combo( xor_c( "Pitch" ), &cond.pitch, aa_pitch, IM_ARRAYSIZE( aa_pitch ) );
            this->combo( xor_c( "Yaw" ), &cond.yaw, aa_yaw, IM_ARRAYSIZE( aa_yaw ) );

            if( cond.yaw == 2 )
            {
              this->checkbox( xor_c( "Randomize speed##distortion" ), &cond.random_speed );

              if( !cond.random_speed )
                this->slider_int( xor_c( "Speed##distortion" ), &cond.distortion_speed, 0, 100 );

              this->slider_int( xor_c( "Range##distortion" ), &cond.distortion_range, -180, 180 );
            }
            else if( cond.yaw == 3 )
              this->slider_int( xor_c( "Offset##distortion" ), &cond.crooked_offset, -180, 180 );

            this->slider_int( xor_c( "Yaw add" ), &cond.yaw_add, -100, 100 );

            this->combo( xor_c( "Yaw jitter" ), &cond.jitter_mode, aa_jitter_yaw, IM_ARRAYSIZE( aa_jitter_yaw ) );
            if( cond.jitter_mode > 0 )
              this->slider_int( xor_c( "Range##jitter" ), &cond.jitter_range, -180, 180 );
          }
        }
        end_child;

        begin_child( xor_c( "Fake angle" ) )
        {
          this->checkbox( xor_c( "Enable##fake_angle" ), &g_cfg.antihit.desync );
          this->combo( xor_c( "LBY Breaker" ), &g_cfg.antihit.desync_type, aa_desync_type, IM_ARRAYSIZE( aa_desync_type ) );
          this->slider_int( xor_c( "LBY delta##desync" ), &g_cfg.antihit.desync_range, 0, 180, xor_c( "%d" ) );
        }
        end_child;
      }
      ImGui::NextColumn( );
      {
        begin_child( xor_c( "Fake lag" ) )
        {
          this->checkbox( xor_c( "Enable##fakelag" ), &g_cfg.antihit.fakelag );

          this->multi_combo( xor_c( "Conditions##fakelag" ), g_cfg.antihit.fakelag_conditions,
            {
              xor_str( "Stand" ),
              xor_str( "Moving" ),
              xor_str( "Air" ),
              xor_str( "Land" ),
            } );

          if( g_cfg.antihit.fakelag )
            this->slider_int( xor_c( "Limit" ), &g_cfg.antihit.fakelag_limit, 1, 16, xor_c( "%d ticks" ) );
        }
        end_child;

        begin_child( xor_c( "Enhancement" ) )
        {
          this->key_bind( xor_c( "Fake walk" ), g_cfg.binds [ sw_b ] );
          this->slider_int( xor_c( "Speed" ), &g_cfg.antihit.fakewalk_speed, 1, 16, xor_c( "%d ticks" ) );
          this->key_bind( xor_c( "Freestanding" ), g_cfg.binds [ edge_b ] );

          this->key_bind( xor_c( "Inverter" ), g_cfg.binds [ inv_b ] );

          this->key_bind( xor_c( "Manual left" ), g_cfg.binds [ left_b ] );
          this->key_bind( xor_c( "Manual right" ), g_cfg.binds [ right_b ] );
          this->key_bind( xor_c( "Manual back" ), g_cfg.binds [ back_b ] );

          this->key_bind( xor_c( "Fake duck" ), g_cfg.binds [ fd_b ] );
        }
        end_child;
      }
    }
    break;
    case 2:
    {
      static int selector = 0;
      this->draw_sub_tabs( selector, visual_tabs );

      static int prev_sub_sel = 0;
      if( selector != prev_sub_sel )
      {
        for( auto& a : item_animations )
        {
          a.second.reset( );
          subtab_alpha = 0.f;
        }

        prev_sub_sel = selector;
      }

      this->create_animation( subtab_alpha, g_cfg.misc.menu && selector == prev_sub_sel, 0.2f, skip_disable | lerp_animation );
      alpha = subtab_alpha * tab_alpha;

      ImGui::SetCursorPosX( 0 );
      ImGui::BeginChild( xor_c( "subtab_vis" ), { }, false );
      switch( selector )
      {
      case 0:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( "General " );
          {
            auto& enemy_esp = g_cfg.visuals.esp [ esp_enemy ];
            this->checkbox( xor_c( "Enable##esp_enemy" ), &enemy_esp.enable );
            this->multi_combo( xor_c( "Elements##esp_enemy" ), enemy_esp.elements,
              {
                xor_str( "Box" ),
                xor_str( "Name" ),
                xor_str( "Health" ),
                xor_str( "Weapon icon" ),
                xor_str( "Weapon" ),
                xor_str( "Ammo" ),
                xor_str( "Flags" ),
                xor_str( "Out of FOV Arrow" ),
                xor_str( "Glow" ),
                xor_str( "Skeleton" ),
              } );

            if( enemy_esp.elements & 1 )
              this->color_picker( xor_c( "Box color##esp_enemy" ), enemy_esp.colors.box );

            if( enemy_esp.elements & 2 )
              this->color_picker( xor_c( "Name color##esp_enemy" ), enemy_esp.colors.name );

            if( enemy_esp.elements & 4 )
              this->color_picker( xor_c( "Health color##esp_enemy" ), enemy_esp.colors.health );

            if( enemy_esp.elements & 8 || enemy_esp.elements & 16 )
              this->color_picker( xor_c( "Weapon color##esp_enemy" ), enemy_esp.colors.weapon );

            if( enemy_esp.elements & 32 )
              this->color_picker( xor_c( "Ammo color##esp_enemy" ), enemy_esp.colors.ammo_bar );

            if( enemy_esp.elements & 128 )
              this->color_picker( xor_c( "OOF Arrow color##esp_enemy" ), enemy_esp.colors.offscreen_arrow );

            if( enemy_esp.elements & 256 )
              this->color_picker( xor_c( "Glow color##esp_enemy" ), enemy_esp.colors.glow );

            if( enemy_esp.elements & 512 )
              this->color_picker( xor_c( "Skeleton color##esp_enemy" ), enemy_esp.colors.skeleton );
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "Chams  " ) );
          {
            static int chams_model = 0;

            this->combo( xor_c( "Model" ), &chams_model, enemy_models, IM_ARRAYSIZE( enemy_models ) );

            auto& enemy_config = g_cfg.visuals.chams [ chams_model ];

            this->checkbox( xor_c( "Enable" ), &enemy_config.enable );
            this->combo( xor_c( "Material" ), &enemy_config.material, vis_chams_type, IM_ARRAYSIZE( vis_chams_type ) );
            this->color_picker( xor_c( "Material color" ), enemy_config.main_color );
            if( enemy_config.material == 4 )
            {
              this->slider_int( xor_c( "Fill" ), &enemy_config.glow_fill, 0, 100, xor_c( "%d%%" ) );
              this->color_picker( xor_c( "Glow color" ), enemy_config.glow_color );
            }

            if( chams_model == 3 )
              this->slider_int( xor_c( "Chams duration" ), &enemy_config.shot_duration, 1, 10, xor_c( "%d s" ) );

            if( enemy_config.material == 6 )
              this->input_text( xor_c( "Sprite path" ), enemy_config.chams_sprite, 128 );
          }
          end_child;
        }
      }
      break;
      case 1:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( xor_c( "Main    " ) )
          {
            this->checkbox( xor_c( "Penetration crosshair" ), &g_cfg.misc.pen_xhair );
            this->checkbox( xor_c( "Force crosshair" ), &g_cfg.misc.snip_crosshair );
            this->checkbox( xor_c( "Fix zoom sensitivity" ), &g_cfg.misc.fix_sensitivity );
          }
          end_child;

          begin_child( xor_c( "Visual adjust    " ) )
          {
            this->checkbox( xor_c( "Glow##local_player" ), &g_cfg.visuals.local_glow );

            if( g_cfg.visuals.local_glow )
              this->color_picker( xor_c( "Glow color##local_esp_glow" ), g_cfg.visuals.local_glow_color );

            this->checkbox( xor_c( "Blend in scope" ), &g_cfg.misc.blend_scope );
            if( g_cfg.misc.blend_scope )
            {
              this->slider_int( xor_c( "Blend amount" ), &g_cfg.misc.scope_amt, 0, 100, xor_c( "%d%%" ) );
              this->slider_int( xor_c( "Attachments blend amount" ), &g_cfg.misc.attachments_amt, 0, 100, xor_c( "%d%%" ) );
            }
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "Chams    " ) )
          {
            static int chams_local_model = 0;

            this->combo( xor_c( "Chams##local" ), &chams_local_model, local_models, IM_ARRAYSIZE( local_models ) );

            auto& local_config = g_cfg.visuals.chams [ chams_local_model + 5 ];

            this->checkbox( xor_c( "Enable##local" ), &local_config.enable );

            this->combo( xor_c( "Material##local" ), &local_config.material, vis_chams_type, IM_ARRAYSIZE( vis_chams_type ) );
            this->color_picker( xor_c( "Material color##local" ), local_config.main_color );
            if( local_config.material == 4 )
            {
              this->slider_int( xor_c( "Fill##local" ), &local_config.glow_fill, 0, 100, xor_c( "%d%%" ) );
              this->color_picker( xor_c( "Glow color##local" ), local_config.glow_color );
            }

            if( local_config.material == 6 )
              this->input_text( xor_c( "Sprite path" ), local_config.chams_sprite, 128 );
          }
          end_child;
        }
      }
      break;
      case 2:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( xor_c( "Weapons & grenades" ) )
          {
            auto& weapon_esp = g_cfg.visuals.esp [ esp_weapon ];
            this->checkbox( xor_c( "Enable##esp_weapon" ), &weapon_esp.enable );
            this->multi_combo( xor_c( "Elements##esp_weapon" ), weapon_esp.elements,
              {
                xor_str( "Box" ),
                xor_str( "Name icon" ),
                xor_str( "Name" ),
                xor_str( "Ammo" ),
                xor_str( "Fire range" ),
                xor_str( "Smoke range" ),
                xor_str( "Glow" ),
              } );

            if( weapon_esp.elements & 1 )
              this->color_picker( xor_c( "Box color##esp_weapon" ), weapon_esp.colors.box );

            if( weapon_esp.elements & 2 || weapon_esp.elements & 4 )
              this->color_picker( xor_c( "Name color##esp_weapon" ), weapon_esp.colors.name );

            if( weapon_esp.elements & 8 )
              this->color_picker( xor_c( "Ammo color##esp_weapon" ), weapon_esp.colors.ammo_bar );

            if( weapon_esp.elements & 16 )
              this->color_picker( xor_c( "Fire color##esp_weapon" ), weapon_esp.colors.molotov_range );

            if( weapon_esp.elements & 32 )
              this->color_picker( xor_c( "Smoke color##esp_weapon" ), weapon_esp.colors.smoke_range );

            if( weapon_esp.elements & 64 )
              this->color_picker( xor_c( "Glow color##esp_weapon" ), weapon_esp.colors.glow );
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "Predictions" ) )
          {
            this->checkbox( xor_c( "Projectile prediction" ), &g_cfg.visuals.grenade_predict );
            if( g_cfg.visuals.grenade_predict )
              this->color_picker( xor_c( "Prediction color" ), g_cfg.visuals.predict_clr );

            this->checkbox( xor_c( "Projectile warning" ), &g_cfg.visuals.grenade_warning );
            this->checkbox( xor_c( "Warning line" ), &g_cfg.visuals.grenade_warning_line );
            this->color_picker( xor_c( "Warning color" ), g_cfg.visuals.warning_clr );
          }
          end_child;
        }
      }
      break;
      case 3:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( xor_c( "Please read on forum how to use this func!" ) )
          {
            this->multi_combo( xor_c( "Material modulation" ), g_cfg.misc.world_material_options, { xor_str( "Texture reflect" ), xor_str( "Force metallic" ), xor_str( "Money mode" ) } );
          }
          end_child;

          begin_child( xor_c( "Other" ) )
          {
            this->combo( xor_c( "Skybox" ), &g_cfg.misc.skybox, skyboxes, IM_ARRAYSIZE( skyboxes ) );
            if( g_cfg.misc.skybox == 23 )
              this->input_text( xor_c( "Sky name" ), g_cfg.misc.skybox_name, 128 );

            this->slider_int( xor_c( "Props alpha" ), &g_cfg.misc.prop_alpha, 0, 100, xor_c( "%d%%" ) );
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "World modulation" ) )
          {
            this->multi_combo( xor_c( "Modifiers" ), g_cfg.misc.world_modulation, { xor_str( "Night mode" ), xor_str( "Sunset mode" ), xor_str( "Fullbright" ) } );

            if( g_cfg.misc.world_modulation & 1 )
            {
              this->color_picker( xor_c( "World color" ), g_cfg.misc.world_clr [ world ], no_alpha_picker );
              this->color_picker( xor_c( "Props color" ), g_cfg.misc.world_clr [ props ], no_alpha_picker );
              this->color_picker( xor_c( "Skybox color" ), g_cfg.misc.world_clr [ sky ], no_alpha_picker );
              this->color_picker( xor_c( "Lights color" ), g_cfg.misc.world_clr [ lights ], no_alpha_picker );
            }

            if( g_cfg.misc.world_modulation & 2 )
            {
              this->slider_int( xor_c( "Sun pitch" ), &g_cfg.misc.sunset_angle.x, -180, 180, xor_c( "%d degrees" ) );
              this->slider_int( xor_c( "Sun yaw" ), &g_cfg.misc.sunset_angle.y, -180, 180, xor_c( "%d degrees" ) );
              this->slider_int( xor_c( "Sun roll" ), &g_cfg.misc.sunset_angle.z, -180, 180, xor_c( "%d degrees" ) );
            }
          }
          end_child;
        }
      }
      break;
      case 4:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( xor_c( "Screen effects" ) )
          {
            this->multi_combo( xor_c( "Removals" ), g_cfg.misc.removals,
              { xor_str( "Scope" ), xor_str( "Visual recoil" ), xor_str( "Post processing" ), xor_str( "Smoke" ), xor_str( "Flash" ), xor_str( "Fog" ), xor_str( "Shadows" ), xor_str( "Viewmodel move" ),
                xor_str( "Landing bob" ) } );
          }
          end_child;

          begin_child( xor_c( "World bloom" ) )
          {
            this->checkbox( xor_c( "Customize bloom" ), &g_cfg.misc.custom_bloom );
            if( g_cfg.misc.custom_bloom )
            {
              this->slider_int( xor_c( "Bloom scale" ), &g_cfg.misc.bloom_scale, 0, 5 );
              this->slider_int( xor_c( "Exposure min" ), &g_cfg.misc.exposure_min, 1, 10 );
              this->slider_int( xor_c( "Exposure max" ), &g_cfg.misc.exposure_max, 1, 10 );
            }
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "World fog" ) )
          {
            this->checkbox( xor_c( "Customize fog" ), &g_cfg.misc.custom_fog );
            if( g_cfg.misc.custom_fog )
            {
              this->slider_int( xor_c( "Fog start" ), &g_cfg.misc.fog_start, 0, 10000 );
              this->slider_int( xor_c( "Fog end" ), &g_cfg.misc.fog_end, 0, 10000 );
              this->slider_int( xor_c( "Density" ), &g_cfg.misc.fog_density, 0, 100, xor_c( "%d%%" ) );
              this->color_picker( xor_c( "Fog color" ), g_cfg.misc.world_clr [ fog ], no_alpha_picker );
            }
          }
          end_child;
        }
      }
      break;
      case 5:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( xor_c( "World camera" ) )
          {
            this->key_bind( xor_c( "Thirdperson" ), g_cfg.binds [ tp_b ] );
            this->slider_int( xor_c( "Distance" ), &g_cfg.misc.thirdperson_dist, 10, 300 );
            this->checkbox( xor_c( "While dead" ), &g_cfg.misc.thirdperson_dead );
          }
          end_child;

          begin_child( xor_c( "Viewmodel  " ) )
          {
            this->slider_int( xor_c( "Viewmodel FOV" ), &g_cfg.misc.fovs [ arms ], 0, 30 );
            this->checkbox( xor_c( "Show viewmodel in scope" ), &g_cfg.misc.viewmodel_scope );
            this->slider_int( xor_c( "Offset X" ), &g_cfg.misc.viewmodel_pos [ 0 ], -20, 20 );
            this->slider_int( xor_c( "Offset Y" ), &g_cfg.misc.viewmodel_pos [ 1 ], -20, 20 );
            this->slider_int( xor_c( "Offset Z" ), &g_cfg.misc.viewmodel_pos [ 2 ], -20, 20 );
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "World view" ) )
          {
            this->slider_int( xor_c( "Aspect ratio" ), &g_cfg.misc.aspect_ratio, 0, 200 );
            this->slider_int( xor_c( "FOV" ), &g_cfg.misc.fovs [ world ], 0, 60 );
            this->slider_int( xor_c( "Zoom FOV" ), &g_cfg.misc.fovs [ zoom ], 0, 100, xor_c( "%d%%" ) );
            this->checkbox( xor_c( "Skip second zoom" ), &g_cfg.misc.skip_second_zoom );
          }
          end_child;
        }
      }
      break;
      }
      ImGui::EndChild( false );

      ImGui::PopClipRect( );
      draw_list->PopClipRect( );
    }
    break;
    case 3:
    {
      static int selector2 = 0;
      this->draw_sub_tabs( selector2, misc_tabs );

      static int prev_sub_sel2 = 0;
      if( selector2 != prev_sub_sel2 )
      {
        for( auto& a : item_animations )
        {
          a.second.reset( );
          subtab_alpha2 = 0.f;
        }
        prev_sub_sel2 = selector2;
      }

      this->create_animation( subtab_alpha2, g_cfg.misc.menu && selector2 == prev_sub_sel2, 0.2f, skip_disable | lerp_animation );
      alpha = subtab_alpha2 * tab_alpha;

      ImGui::SetCursorPosX( 0 );
      ImGui::BeginChild( xor_c( "subtab_misc" ), { }, false );
      switch( selector2 )
      {
      case 0:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( xor_c( "Main" ) )
          {
            this->checkbox( xor_c( "Auto jump" ), &g_cfg.misc.auto_jump );

            this->checkbox( xor_c( "Auto strafe" ), &g_cfg.misc.auto_strafe );
            this->slider_int( xor_c( "Strafe smooth" ), &g_cfg.misc.strafe_smooth, 0, 100, xor_c( "%d%%" ) );

            this->checkbox( xor_c( "Fast stop" ), &g_cfg.misc.fast_stop );
            this->checkbox( xor_c( "Slide walk" ), &g_cfg.misc.slide_walk );
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "Helpers" ) )
          {
            this->key_bind( xor_c( "Auto peek" ), g_cfg.binds [ ap_b ] );
            this->color_picker( xor_c( "Start color##peek" ), g_cfg.misc.autopeek_clr );
            this->color_picker( xor_c( "Move color##peek" ), g_cfg.misc.autopeek_clr_back );
            this->checkbox( xor_c( "Return on key release" ), &g_cfg.misc.retrack_peek );
          }
          end_child;
        }
      }
      break;
      case 1:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( xor_c( "Tracers" ) )
          {
            this->checkbox( xor_c( "Bullet impacts" ), &g_cfg.misc.impacts );
            this->color_picker( xor_c( "Server color" ), g_cfg.misc.server_clr );
            this->color_picker( xor_c( "Client color" ), g_cfg.misc.client_clr );
            this->slider_int( xor_c( "Size" ), &g_cfg.misc.impact_size, 5, 15 );

            this->multi_combo( xor_c( "Bullet tracers" ), g_cfg.misc.tracers, { xor_str( "Enemy" ), xor_str( "Team" ), xor_str( "Local" ) } );

            this->combo( xor_c( "Tracer type" ), &g_cfg.misc.tracer_type, tracers, IM_ARRAYSIZE( tracers ) );

            if( g_cfg.misc.tracers & 1 )
              this->color_picker( xor_c( "Enemy tracer" ), g_cfg.misc.trace_clr [ 0 ] );

            if( g_cfg.misc.tracers & 2 )
              this->color_picker( xor_c( "Team tracer" ), g_cfg.misc.trace_clr [ 1 ] );

            if( g_cfg.misc.tracers & 4 )
              this->color_picker( xor_c( "Local tracer" ), g_cfg.misc.trace_clr [ 2 ] );
          }
          end_child;

          begin_child( xor_c( "Event log" ) )
          {
            this->checkbox( xor_c( "Enable##eventlog" ), &g_cfg.visuals.eventlog.enable );
            this->multi_combo( xor_c( "Type##eventlog" ), g_cfg.visuals.eventlog.logs, { xor_str( "Hits" ), xor_str( "Misses" ), xor_str( "Debug" ), xor_str( "Purchases" ), xor_str( "Bomb plant" ) } );
            this->checkbox( xor_c( "Filter console##eventlog" ), &g_cfg.visuals.eventlog.filter_console );
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "Hit indicators" ) )
          {
            this->multi_combo( xor_c( "Hitmarker" ), g_cfg.misc.hitmarker, { xor_str( "On enemy" ), xor_str( "On screen" ) } );
            this->color_picker( xor_c( "Hitmarker color" ), g_cfg.misc.hitmarker_clr );

            this->combo( xor_c( "Hitsound" ), &g_cfg.misc.sound, hitsound, IM_ARRAYSIZE( hitsound ) );
            this->slider_int( xor_c( "Volume" ), &g_cfg.misc.sound_volume, 0, 100, xor_c( "%d%%" ) );

            if( g_cfg.misc.sound == 2 )
              this->input_text( xor_c( "Sound name" ), g_cfg.misc.sound_name, 128 );

            this->checkbox( xor_c( "Damage indicator" ), &g_cfg.misc.damage );
            this->color_picker( xor_c( "Indicator color" ), g_cfg.misc.damage_clr );
          }
          end_child;

          begin_child( xor_c( "Buy bot" ) )
          {
            this->checkbox( xor_c( "Enable##buybot" ), &g_cfg.misc.buybot.enable );
            this->combo( xor_c( "Primary weapon##buybot" ), &g_cfg.misc.buybot.main_weapon, buybot_main, IM_ARRAYSIZE( buybot_main ) );
            this->combo( xor_c( "Secondary weapon##buybot" ), &g_cfg.misc.buybot.second_weapon, buybot_second, IM_ARRAYSIZE( buybot_second ) );

            this->multi_combo( xor_c( "Items##buybot" ), g_cfg.misc.buybot.other_items,
              { xor_str( "Head armor" ), xor_str( "Armor" ), xor_str( "HE grenade" ), xor_str( "Molotov" ), xor_str( "Smoke" ), xor_str( "Taser" ), xor_str( "Defuse kit" ) } );
          }
          end_child;
        }
      }
      break;
      case 2:
      {
        ImGui::Columns( 2, 0, false );
        ImGui::SetColumnOffset( 1, 270 );
        {
          begin_child( xor_c( "Hud" ) )
          {
            this->checkbox( xor_c( "Force engine radar" ), &g_cfg.misc.force_radar );
            this->checkbox( xor_c( "Preserve killfeed" ), &g_cfg.misc.preverse_killfeed );
            this->checkbox( xor_c( "Unlock inventory" ), &g_cfg.misc.unlock_inventory );
            this->checkbox( xor_c( "Remove server ads" ), &g_cfg.misc.remove_ads );
          }
          end_child;

          begin_child( xor_c( "Menu" ) )
          {
            this->color_picker( xor_c( "Accent color" ), g_cfg.misc.ui_color );
            this->multi_combo( xor_c( "Indicators##menu" ), g_cfg.misc.menu_indicators, { xor_str( "Keybind list" ), xor_str( "Bomb" ), xor_str( "Watermark" ) } );
          }
          end_child;
        }
        ImGui::NextColumn( );
        {
          begin_child( xor_c( "Changers" ) )
          {
            this->combo( xor_c( "Ragdoll gravity" ), &g_cfg.misc.ragdoll_gravity, ragdoll_gravities, IM_ARRAYSIZE( ragdoll_gravities ) );

            this->multi_combo( xor_c( "Animation changers" ), g_cfg.misc.animation_changes, { xor_str( "Zero pitch on land" ), xor_str( "Break leg move" ), xor_str( "Movement lean" ) } );

            this->checkbox( xor_c( "Clantag changer" ), &g_cfg.misc.clantag );
          }
          end_child;

          begin_child( xor_c( "Spoofers" ) )
          {
            this->checkbox( xor_c( "Spoof sv_cheats" ), &g_cfg.misc.spoof_sv_cheats );
            this->checkbox( xor_c( "Unlock hidden cvars" ), &g_cfg.misc.unlock_hidden_cvars );
            this->checkbox( xor_c( "Bypass sv_pure" ), &g_cfg.misc.bypass_sv_pure );
          }
          end_child;
        }
      }
      break;
      }
      ImGui::EndChild( false );

      ImGui::PopClipRect( );
      draw_list->PopClipRect( );
    }
    break;
    case 4:
    {
      ImGui::Columns( 2, 0, false );
      ImGui::SetColumnOffset( 1, 270 );
      {
        begin_child( xor_c( "Choose your weapon     " ) )
        {
          this->combo( xor_c( "Painting up...##skins" ), &g_cfg.skins.group_type, skin_weapon_configs, IM_ARRAYSIZE( skin_weapon_configs ) );

          g_cfg.skins.skin_weapon [ g_cfg.skins.group_type ].knife_model = std::clamp( g_cfg.skins.skin_weapon [ g_cfg.skins.group_type ].knife_model, 0, 10 );

          if( g_cfg.skins.group_type == weapon_cfg_knife )
            this->combo( xor_c( "Model  " ), &g_cfg.skins.skin_weapon [ g_cfg.skins.group_type ].knife_model, knife_models, IM_ARRAYSIZE( knife_models ) );
        }
        end_child;

        begin_child( xor_c( "Gloves" ) )
        {
          this->combo( xor_c( "Glove model" ), &g_cfg.skins.model_glove, glove_models, IM_ARRAYSIZE( glove_models ) );
          this->combo( xor_c( "Glove skin" ), &g_cfg.skins.glove_skin, glove_skins, IM_ARRAYSIZE( glove_skins ) );
        }
        end_child;
      }
      ImGui::NextColumn( );
      {
        static std::vector< std::string > skin_names{ };
        if( skin_names.size( ) < skin_changer::paint_kits.size( ) )
        {
          for( auto& s : skin_changer::paint_kits )
            skin_names.emplace_back( s.first );
        }

        auto& config = g_cfg.skins.skin_weapon [ g_cfg.skins.group_type ];
        auto skin_list = skin_changer::paint_kits.size( ) <= 0 ? empty_list : skin_names;
        std::string skin_list_child_name = xor_str( "Skins | " ) + std::to_string( skin_list.size( ) ) + xor_str( " total" );

        begin_child( skin_list_child_name.c_str( ) )
        {
          this->checkbox( xor_c( "Enable" ), &config.enable );

          this->input_text( xor_c( "Skins search" ), config.skins_search, 256 );
          this->listbox( xor_c( "skinlist_____" ), &config.skin, skin_list, 17, config.skins_search );
        }
        end_child;
      }
    }
    break;
    case 5:
    {
      static int config_select = -1;
      static char config_name [ 256 ]{ };

      bool wrong_config = config_select == -1 || config_list.size( ) <= 0 || config_select > config_list.size( ) - 1;

      int cfg_flags = 0;
      if( wrong_config )
        cfg_flags |= button_wait;

      ImGui::Columns( 2, 0, false );
      ImGui::SetColumnOffset( 1, 270 );
      {
        begin_child( xor_c( "General   " ) )
        {
          this->input_text( xor_c( "Config name" ), config_name, 256 );

          int len = std::strlen( config_name );

          this->button(
            xor_c( "Create" ),
            []( )
            {
              config::create( config_name );
              refresh_configs( );
            },
            len <= 0 ? button_wait : 0 );

          this->button( xor_c( "Refresh" ), []( ) { refresh_configs( ); } );

          this->button( xor_c( "Open configs folder" ),
            []( )
            {
              static TCHAR path [ MAX_PATH ]{ };
              std::string folder = "";

              if( SUCCEEDED( SHGetFolderPathA( NULL, CSIDL_PERSONAL, NULL, 0, path ) ) )
              {
                folder = std::string( path ) + xor_c( "\\airflow\\" );
              }
              ShellExecuteA( NULL, NULL, folder.c_str( ), NULL, NULL, SW_SHOWNORMAL );
            } );
        }
        end_child;

        std::string config_child_name = xor_str( "Config: " ) + ( wrong_config ? xor_str( "None" ) : config_list [ config_select ] );

        begin_child( config_child_name.c_str( ) )
        {
          this->button(
            xor_c( "Load" ),
            []( )
            {
              config::load( config_list [ config_select ] );
              if( !g_ctx.loading_config )
                g_ctx.loading_config = true;
            },
            cfg_flags );

          this->button(
            xor_c( "Save" ), []( ) { config::save( config_list [ config_select ] ); }, cfg_flags );

          this->button(
            xor_c( "Reset" ),
            []( )
            {
              g_cfg = configs_t( );
              config::save( config_list [ config_select ] );
            },
            cfg_flags );

          int delete_flags = button_danger;
          if( wrong_config )
            delete_flags |= button_wait;

          this->button(
            xor_c( "Delete" ),
            []( )
            {
              config::erase( config_list [ config_select ] );
              refresh_configs( );
            },
            delete_flags );

#ifdef _DEBUG
          this->button( xor_c( "Unload" ), []( ) { g_ctx.uninject = true; } );
#endif
        }
        end_child;
      }
      ImGui::NextColumn( );
      {
        static char config_search [ 256 ];

        auto list = config_list.size( ) <= 0 ? empty_list : config_list;
        std::string config_list_child_name = xor_str( "Configs | " ) + std::to_string( list.size( ) ) + xor_str( " total" );

        begin_child( config_list_child_name.c_str( ) )
        {
          this->input_text( xor_c( "Config search" ), config_search, 256 );
          this->listbox( xor_c( "configlist___" ), &config_select, list, 19, config_search );
        }
        end_child;
      }
    }
    break;
    }
  }
  ImGui::EndChild( false );

  alpha = old_alpha;
  ImGui::PopClipRect( );
  ImGui::SetCursorPos( prev_pos );
}