#include <algorithm>
#include <fstream>

#include "skins.h"
#include "../ragebot/rage_tools.h"

#include "../../base/tools/memory/memory.h"

namespace skin_changer
{
  bool call_force_update = false;
  bool in_force_update = false;
  bool update_hud = false;
  float last_update_time = 0.f;

  constexpr auto mask_flags = 0x10000;
  constexpr auto max_knifes = 11;
  constexpr auto max_gloves = 7;
  const char* default_mask = ( "models/player/holiday/facemasks/facemask_battlemask.mdl" );

  std::unordered_map< std::string, int > weapon_indexes{ };

  std::array< std::string, max_knifes - 1 > knife_models{
    xor_str( "models/weapons/v_knife_bayonet.mdl" ),
    xor_str( "models/weapons/v_knife_flip.mdl" ),
    xor_str( "models/weapons/v_knife_gut.mdl" ),
    xor_str( "models/weapons/v_knife_karam.mdl" ),
    xor_str( "models/weapons/v_knife_m9_bay.mdl" ),
    xor_str( "models/weapons/v_knife_tactical.mdl" ),
    xor_str( "models/weapons/v_knife_falchion_advanced.mdl" ),
    xor_str( "models/weapons/v_knife_survival_bowie.mdl" ),
    xor_str( "models/weapons/v_knife_butterfly.mdl" ),
    xor_str( "models/weapons/v_knife_push.mdl" ),
  };

  std::array< std::string, max_knifes - 1 > world_knife_models{
    xor_str( "models/weapons/w_knife_bayonet.mdl" ),
    xor_str( "models/weapons/w_knife_flip.mdl" ),
    xor_str( "models/weapons/w_knife_gut.mdl" ),
    xor_str( "models/weapons/w_knife_karam.mdl" ),
    xor_str( "models/weapons/w_knife_m9_bay.mdl" ),
    xor_str( "models/weapons/w_knife_tactical.mdl" ),
    xor_str( "models/weapons/w_knife_falchion_advanced.mdl" ),
    xor_str( "models/weapons/w_knife_survival_bowie.mdl" ),
    xor_str( "models/weapons/w_knife_butterfly.mdl" ),
    xor_str( "models/weapons/w_knife_push.mdl" ),
  };

  std::array< std::string, max_gloves > gloves{
    xor_str( "models/weapons/w_models/arms/w_glove_bloodhound.mdl" ),
    xor_str( "models/weapons/w_models/arms/w_glove_sporty.mdl" ),
    xor_str( "models/weapons/w_models/arms/w_glove_slick.mdl" ),
    xor_str( "models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl" ),
    xor_str( "models/weapons/w_models/arms/w_glove_motorcycle.mdl" ),
    xor_str( "models/weapons/w_models/arms/w_glove_specialist.mdl" ),
    xor_str( "models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl" ),
  };

  struct knife_id_t
  {
    short index{ };
    std::string name{ };
  };

  std::array< knife_id_t, max_knifes > knifes{
    knife_id_t{ weapon_none, xor_str( "def" ) },
    { weapon_knife_bayonet, xor_str( "bayonet" ) },
    { weapon_knife_flip, xor_str( "flip" ) },
    { weapon_knife_gut, xor_str( "gut" ) },
    { weapon_knife_karambit, xor_str( "karambit" ) },
    { weapon_knife_m9_bayonet, xor_str( "m9 bayonet" ) },
    { weapon_knife_tactical, xor_str( "tactical" ) },
    { weapon_knife_falchion, xor_str( "falchion" ) },
    { weapon_knife_survival_bowie, xor_str( "bowie" ) },
    { weapon_knife_butterfly, xor_str( "butterfly" ) },
    { weapon_knife_push, xor_str( "push" ) },
  };

  enum knife_sequcence_t : int
  {
    sequence_default_draw = 0,
    sequence_default_idle1 = 1,
    sequence_default_idle2 = 2,
    sequence_default_light_miss1 = 3,
    sequence_default_light_miss2 = 4,
    sequence_default_heavy_miss1 = 9,
    sequence_default_heavy_hit1 = 10,
    sequence_default_heavy_backstab = 11,
    sequence_default_lookat01 = 12,

    sequence_butterfly_draw = 0,
    sequence_butterfly_draw2 = 1,
    sequence_butterfly_lookat01 = 13,
    sequence_butterfly_lookat03 = 15,

    sequence_falchion_idle1 = 1,
    sequence_falchion_heavy_miss1 = 8,
    sequence_falchion_heavy_miss1_noflip = 9,
    sequence_falchion_lookat01 = 12,
    sequence_falchion_lookat02 = 13,

    sequence_css_lookat01 = 14,
    sequence_css_lookat02 = 15,

    sequence_daggers_idle1 = 1,
    sequence_daggers_light_miss1 = 2,
    sequence_daggers_light_miss5 = 6,
    sequence_daggers_heavy_miss2 = 11,
    sequence_daggers_heavy_miss1 = 12,

    sequence_bowie_idle1 = 1,
  };

  struct weapon_info
  {
    constexpr weapon_info( const char* model, const char* icon = nullptr, int animindex = -1 ): model( model ), icon( icon ), animindex( animindex )
    {
    }
    const char* model;
    const char* icon;
    int animindex;
  };

  const weapon_info* get_weapon_info( int defindex )
  {
    const static std::map< int, weapon_info > Info = { { weapon_knife, { xor_c( "models/weapons/v_knife_default_ct.mdl" ), xor_c( "knife_default_ct" ), 2 } },
      { weapon_knife_t, { xor_c( "models/weapons/v_knife_default_t.mdl" ), xor_c( "knife_t" ), 12 } }, { weapon_knife_bayonet, { xor_c( "models/weapons/v_knife_bayonet.mdl" ), xor_c( "bayonet" ), 0 } },
      { weapon_knife_flip, { xor_c( "models/weapons/v_knife_flip.mdl" ), xor_c( "knife_flip" ), 4 } }, { weapon_knife_gut, { xor_c( "models/weapons/v_knife_gut.mdl" ), xor_c( "knife_gut" ), 5 } },
      { weapon_knife_karambit, { xor_c( "models/weapons/v_knife_karam.mdl" ), xor_c( "knife_karambit" ), 7 } }, { weapon_knife_m9_bayonet, { xor_c( "models/weapons/v_knife_m9_bay.mdl" ), xor_c( "knife_m9_bayonet" ), 8 } },
      { weapon_knife_tactical, { xor_c( "models/weapons/v_knife_tactical.mdl" ), xor_c( "knife_tactical" ) } }, { weapon_knife_falchion, { xor_c( "models/weapons/v_knife_falchion_advanced.mdl" ), xor_c( "knife_falchion" ), 3 } },
      { weapon_knife_survival_bowie, { xor_c( "models/weapons/v_knife_survival_bowie.mdl" ), xor_c( "knife_survival_bowie" ), 11 } },
      { weapon_knife_butterfly, { xor_c( "models/weapons/v_knife_butterfly.mdl" ), xor_c( "knife_butterfly" ), 1 } }, { weapon_knife_push, { xor_c( "models/weapons/v_knife_push.mdl" ), xor_c( "knife_push" ), 9 } },
      { glove_studded_bloodhound, { xor_c( "models/weapons/w_models/arms/w_glove_bloodhound.mdl" ) } }, { glove_t, { xor_c( "models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl" ) } },
      { glove_ct, { xor_c( "models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl" ) } }, { glove_sporty, { xor_c( "models/weapons/w_models/arms/w_glove_sporty.mdl" ) } },
      { glove_slick, { xor_c( "models/weapons/w_models/arms/w_glove_slick.mdl" ) } }, { glove_leather_handwraps, { xor_c( "models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl" ) } },
      { glove_motorcycle, { xor_c( "models/weapons/w_models/arms/w_glove_motorcycle.mdl" ) } }, { glove_specialist, { xor_c( "models/weapons/w_models/arms/w_glove_specialist.mdl" ) } },
      { glove_studded_hydra, { xor_c( "models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl" ) } }, { 521, { xor_c( "models/weapons/v_knife_outdoor.mdl" ), xor_c( "knife_outdoor" ), 14 } },
      { 518, { xor_c( "models/weapons/v_knife_canis.mdl" ), xor_c( "knife_canis" ), 14 } }, { 517, { xor_c( "models/weapons/v_knife_cord.mdl" ), xor_c( "knife_cord" ), 14 } },
      { 525, { xor_c( "models/weapons/v_knife_skeleton.mdl" ), xor_c( "knife_skeleton" ), 14 } }, { 503, { xor_c( "models/weapons/v_knife_css.mdl" ), xor_c( "knife_css" ), 14 } } };

    const auto entry = Info.find( defindex );
    return entry == end( Info ) ? nullptr : &entry->second;
  }

  __forceinline void precache( const char* name )
  {
    if( name == "" )
      return;

    if( name == nullptr )
      return;

    auto modelprecache = interfaces::network_string_table_container->find_table( xor_c( "modelprecache" ) );
    if( !modelprecache )
      return;

    auto idx = modelprecache->add_string( false, name );
    if( idx == -1 )
      return;
  }

  __forceinline void init_parser( )
  {
    std::string items{ };

    if( std::ifstream file{ xor_c( "csgo/scripts/items/items_game_cdn.txt" ) } )
      items = { std::istreambuf_iterator{ file }, {} };

    if( items.empty( ) )
      return;

    for( int i = 0; i <= interfaces::item_schema->paint_kits.last_element; ++i )
    {
      const auto& paint_kit = interfaces::item_schema->paint_kits.memory [ i ]._value;
      if( paint_kit->id == 9001 || paint_kit->id >= 10000 )
        continue;

      auto name = string_convert::to_string( interfaces::localize->find_safe( paint_kit->item_name.buffer + 1 ) );
      if( name == "-" )
        continue;

      paint_kits.emplace_back( std::make_pair( name, paint_kit->id ) );
    }

    std::sort( paint_kits.begin( ), paint_kits.end( ), [ & ]( const std::pair< std::string, int >& a, const std::pair< std::string, int >& b ) { return a.first < b.first; } );
  }

  static auto random_sequence( const int low, const int high ) -> int
  {
    return rand( ) % ( high - low + 1 ) + low;
  }

  __forceinline int correct_sequence( const short& index, const int sequence )
  {
    switch( index )
    {
    case weapon_knife_butterfly:
    {
      switch( sequence )
      {
      case sequence_default_draw:
        return random_sequence( sequence_butterfly_draw, sequence_butterfly_draw2 );
      case sequence_default_lookat01:
        return random_sequence( sequence_butterfly_lookat01, sequence_butterfly_lookat03 );
      default:
        return sequence + 1;
      }
    }
    case weapon_knife_falchion:
    {
      switch( sequence )
      {
      case sequence_default_idle2:
        return sequence_falchion_idle1;
      case sequence_default_heavy_miss1:
        return random_sequence( sequence_falchion_heavy_miss1, sequence_falchion_heavy_miss1_noflip );
      case sequence_default_lookat01:
        return random_sequence( sequence_falchion_lookat01, sequence_falchion_lookat02 );
      case sequence_default_draw:
      case sequence_default_idle1:
        return sequence;
      default:
        return sequence - 1;
      }
    }
    case weapon_knife_push:
    {
      switch( sequence )
      {
      case sequence_default_idle2:
        return sequence_daggers_idle1;
      case sequence_default_light_miss1:
      case sequence_default_light_miss2:
        return random_sequence( sequence_daggers_light_miss1, sequence_daggers_light_miss5 );
      case sequence_default_heavy_miss1:
        return random_sequence( sequence_daggers_heavy_miss2, sequence_daggers_heavy_miss1 );
      case sequence_default_heavy_hit1:
      case sequence_default_heavy_backstab:
      case sequence_default_lookat01:
        return sequence + 3;
      case sequence_default_draw:
      case sequence_default_idle1:
        return sequence;
      default:
        return sequence + 2;
      }
    }
    case weapon_knife_survival_bowie:
    {
      switch( sequence )
      {
      case sequence_default_draw:
      case sequence_default_idle1:
        return sequence;
      case sequence_default_idle2:
        return sequence_bowie_idle1;
      default:
        return sequence - 1;
      }
    }
    default:
      return sequence;
    }
  }

  short item_def_glove( )
  {
    switch( g_cfg.skins.model_glove )
    {
    case 1:
      return glove_studded_bloodhound;
      break;
    case 2:
      return glove_sporty;
      break;
    case 3:
      return glove_slick;
      break;
    case 4:
      return glove_leather_handwraps;
      break;
    case 5:
      return glove_motorcycle;
      break;
    case 6:
      return glove_specialist;
      break;
    case 7:
      return glove_studded_hydra;
      break;
    default:
      break;
    }
  }

  int glove_skins_id( )
  {
    switch( g_cfg.skins.glove_skin )
    {
    case 0:
      return 10006;
      break;
    case 1:
      return 10007;
      break;
    case 2:
      return 10008;
      break;
    case 3:
      return 10009;
      break;
    case 4:
      return 10010;
      break;
    case 5:
      return 10013;
      break;
    case 6:
      return 10015;
      break;
    case 7:
      return 10016;
      break;
    case 8:
      return 10018;
      break;
    case 9:
      return 10019;
      break;
    case 10:
      return 10021;
      break;
    case 11:
      return 10024;
      break;
    case 12:
      return 10026;
      break;
    case 13:
      return 10027;
      break;
    case 14:
      return 10028;
      break;
    case 15:
      return 10030;
      break;
    case 16:
      return 10033;
      break;
    case 17:
      return 10034;
      break;
    case 18:
      return 10035;
      break;
    case 19:
      return 10036;
      break;
    case 20:
      return 10037;
      break;
    case 21:
      return 10038;
      break;
    case 22:
      return 10039;
      break;
    case 23:
      return 10040;
      break;
    case 24:
      return 10041;
      break;
    case 25:
      return 10042;
      break;
    case 26:
      return 10043;
      break;
    case 27:
      return 10044;
      break;
    case 28:
      return 10045;
      break;
    case 29:
      return 10046;
      break;
    case 30:
      return 10047;
      break;
    case 31:
      return 10048;
      break;
    case 32:
      return 10049;
      break;
    case 33:
      return 10050;
      break;
    case 34:
      return 10051;
      break;
    case 35:
      return 10052;
      break;
    case 36:
      return 10053;
      break;
    case 37:
      return 10054;
      break;
    case 38:
      return 10055;
      break;
    case 39:
      return 10056;
      break;
    case 40:
      return 10057;
      break;
    case 41:
      return 10058;
      break;
    case 42:
      return 10059;
      break;
    case 43:
      return 10060;
      break;
    case 44:
      return 10061;
      break;
    case 45:
      return 10062;
      break;
    case 46:
      return 10063;
      break;
    case 47:
      return 10064;
      break;
    default:
      break;
    }
  }

  static __forceinline auto get_wearable_create_fn( ) -> create_client_class_fn
  {
    auto classes = interfaces::client->get_client_classes( );

    while( classes->class_id != CEconWearable )
      classes = classes->next_ptr;

    return classes->create_fn;
  }

  static __forceinline c_basecombatweapon* make_glove( int entry, int serial ) noexcept
  {
    get_wearable_create_fn( )( entry, serial );

    auto glove = ( c_basecombatweapon* )( interfaces::entity_list->get_entity( entry ) );

    if( !glove )
      return nullptr;

    static auto Fn = g_memory->find_pattern( modules::client, xor_c( "55 8B EC 83 E4 F8 51 53 56 57 8B F1" ) );
    static auto set_abs_origin = Fn.as< void( __thiscall* )( void*, const vector3d& ) >( );

    set_abs_origin( glove, vector3d( 16384.0f, 16384.0f, 16384.0f ) );
    return glove;
  }

  __forceinline bool set_paint_kit( c_basecombatweapon* weapon, skin_weapon_t& cfg )
  {
    cfg.knife_model = std::clamp( cfg.knife_model, 0, max_knifes );

    int skin = std::clamp( cfg.skin, 0, ( int )paint_kits.size( ) - 1 );

    auto& skin_data = paint_kits [ skin ];

    auto client_class = weapon->get_client_class( );
    if( !client_class )
      return false;

    weapon->fallback_paint_kit( ) = skin_data.second;

    weapon->original_owner_xuid_low( ) = 0;
    weapon->original_owner_xuid_high( ) = 0;
    weapon->fallback_wear( ) = 0.001f;
    weapon->item_id_high( ) = -1;

    if( client_class->class_id == CKnife )
    {
      if( cfg.knife_model > 0 )
      {
        auto model_index = interfaces::model_info->get_model_index( knife_models.at( cfg.knife_model - 1 ).data( ) );
        auto world_model_index = interfaces::model_info->get_model_index( world_knife_models.at( cfg.knife_model - 1 ).data( ) );

        auto item_index = knifes [ cfg.knife_model ].index;

        if( item_index != weapon->item_definition_index( ) )
        {
          weapon->item_definition_index( ) = item_index;

          weapon->set_model_index( model_index );

          const auto networkable = weapon->get_networkable_entity( );

          using fn = void( __thiscall* )( void*, const int );
          g_memory->getvfunc< fn >( networkable, 6 )( networkable, 0 );
        }

        auto view_model = g_ctx.local->get_view_model( );
        if( view_model )
        {
          auto world_weapon = view_model->get_view_model_weapon( );
          if( world_weapon && world_weapon == weapon )
          {
            auto world_model = weapon->get_weapon_world_model( );
            if( world_model )
            {
              view_model->set_model_index( model_index );
              world_model->set_model_index( world_model_index );
            }
          }
        }
      }

      auto updated = cfg.old_skin != cfg.skin;

      cfg.old_skin = cfg.skin;

      if( !updated )
        updated = cfg.old_model != cfg.knife_model;

      cfg.old_model = cfg.knife_model;

      return updated;
    }

    const auto updated = cfg.old_skin != cfg.skin;

    cfg.old_skin = cfg.skin;

    return updated;
  }

  __forceinline void force_update_skin( c_basecombatweapon* weapon )
  {
    *( bool* )( ( uintptr_t )weapon + 0x32DD ) = weapon->fallback_paint_kit( ) <= 0;

    auto& vec0 = *( c_utlvector< ret_counted_t* >* )( ( uintptr_t )weapon + offsets::m_Item + 0x14 );
    for( int i{ }; i < vec0.m_size; ++i )
      vec0.m_memory.base( ) [ i ] = nullptr;

    vec0.m_size = 0;

    auto& vec1 = *( c_utlvector< ret_counted_t* >* )( ( uintptr_t )weapon + 0x9DC );
    for( int i{ }; i < vec1.m_size; ++i )
      vec1.m_memory.base( ) [ i ] = nullptr;

    vec1.m_size = 0;

    auto& vec2 = *( c_utlvector< ret_counted_t* >* )( ( uintptr_t )weapon + offsets::m_Item + 0x220 );
    for( int i{ }; i < vec2.m_size; ++i )
    {
      auto& element = vec2.m_memory.base( ) [ i ];
      if( !element )
        continue;

      element->unref( );
      element = nullptr;
    }

    vec2.m_size = 0;

    const auto networkable = weapon->get_networkable_entity( );
    networkable->post_update( 0 );
    networkable->data_changed( 0 );

    auto hud_selection = ( void* )func_ptrs::find_hud_element( *patterns::get_hud_ptr.as< uintptr_t** >( ), xor_str( "SFWeaponSelection" ).c_str( ) );
    if( !hud_selection )
      return;

    func_ptrs::show_and_update_selection( hud_selection, 0, weapon, false );
  }

  __forceinline void glove_changer( )
  {
    static int old_kit = -1;
    static short old_glove = -1;

    if( !g_ctx.in_game )
    {
      old_kit = -1;
      old_glove = -1;
      return;
    }

    player_info_t info{ };
    interfaces::engine->get_player_info( g_ctx.local->index( ), &info );

    static int m_nBody = find_in_datamap( g_ctx.local->get_pred_desc_map( ), xor_c( "m_nBody" ) );

    static auto equip_item = g_memory->find_pattern( modules::client, xor_c( "55 8B EC 83 EC 10 53 8B 5D 08 57 8B F9" ) ).as< int( __thiscall* )( void*, void* ) >( );
    static auto init_attributes = g_memory->find_pattern( modules::client, xor_c( "55 8B EC 83 E4 F8 83 EC 0C 53 56 8B F1 8B 86" ) ).as< int( __thiscall* )( void* ) >( );

    if( g_cfg.skins.model_glove == 0 )
    {
      if( old_kit != -1 || old_glove != -1 )
      {
        call_force_update = true;

        old_kit = -1;
        old_glove = -1;
      }

      return;
    }

    static auto glove_handle = uint32_t( 0 );
    auto wearables = g_ctx.local->wearables( );
    auto glove = ( c_basecombatweapon* )( interfaces::entity_list->get_entity_handle( wearables [ 0 ] ) );

    if( !glove )
    {
      auto our_glove = ( c_basecombatweapon* )( interfaces::entity_list->get_entity_handle( glove_handle ) );

      if( our_glove )
      {
        wearables [ 0 ] = glove_handle;
        glove = our_glove;
      }
    }

    if( !g_ctx.local->is_alive( ) )
    {
      if( glove )
      {
        glove->set_destroyed_on_recreate_entities( );
        glove->release( );
      }

      old_glove = -1;
      old_kit = -1;
      return;
    }

    if( !g_ctx.weapon )
      return;

    auto glove_index = item_def_glove( );
    auto glove_kit = glove_skins_id( );

    if( glove_index )
    {
      if( !glove )
      {
        auto entry = interfaces::entity_list->get_highest_ent_index( ) + 1;
        auto serial = rand( ) % 0x1000;

        glove = make_glove( entry, serial );
        wearables [ 0 ] = entry | serial << 16;
        glove_handle = wearables [ 0 ];
      }

      static int force_update_count = 0;

      *reinterpret_cast< int* >( uintptr_t( glove ) + 0x64 ) = -1;

      auto& paint_kit = glove->fallback_paint_kit( );

      if( glove_kit && glove_kit != paint_kit )
      {
        paint_kit = glove_kit;

        if( old_kit != glove_kit )
        {
          force_update_count = 0;
          old_kit = glove_kit;
        }

        if( g_cfg.misc.menu && force_update_count < 1 )
        {
          ++force_update_count;
          call_force_update = true;
        }
      }

      glove->fallback_wear( ) = 0.001f;
      glove->item_id_high( ) = -1;

      auto& definition_index = glove->item_definition_index( );

      const auto& replacement_item = get_weapon_info( glove_index );

      if( glove_index && glove_index != definition_index )
      {
        if( !replacement_item )
          return;

        definition_index = glove_index;

        if( weapon_indexes.find( replacement_item->model ) == weapon_indexes.end( ) )
          weapon_indexes.emplace( replacement_item->model, interfaces::model_info->get_model_index( replacement_item->model ) );

        glove->set_model_index( weapon_indexes.at( replacement_item->model ) );
        const auto networkable = glove->get_networkable_entity( );

        using fn = void( __thiscall* )( void*, const int );
        g_memory->getvfunc< fn >( networkable, 6 )( networkable, 0 );

        if( old_glove != glove_index )
        {
          force_update_count = 0;
          old_glove = glove_index;
        }

        // ghetto fix
        // it gets 0 item definition index and call force update multiple times
        // but glove is still applied
        // idk how to fix it, let's just update it once then
        if( g_cfg.misc.menu && force_update_count < 1 )
        {
          ++force_update_count;
          call_force_update = true;
        }
      }
    }
  }

  __forceinline void on_postdataupdate_start( int stage )
  {
    if( stage == frame_net_update_end && interfaces::client_state->delta_tick > 0 )
      in_force_update = false;

    if( !g_ctx.local || !g_ctx.local->is_alive( ) )
      return;

    if( stage == frame_net_update_postdataupdate_end )
      glove_changer( );

    if( !g_ctx.weapon )
      return;

    if( stage != frame_net_update_postdataupdate_start )
      return;

    auto weapon_list = g_ctx.local->get_weapons( );

    for( auto weapon : weapon_list )
    {
      if( !weapon )
        continue;

      auto& skin_cfg = g_cfg.skins.skin_weapon [ cheat_tools::get_legit_tab( weapon ) ];
      if( !skin_cfg.enable )
      {
        if( skin_cfg.old_skin != -1 )
        {
          call_force_update = true;
          skin_cfg.old_skin = -1;
        }

        continue;
      }

      if( weapon->is_misc_weapon( ) && !weapon->is_knife( ) )
        continue;

      if( !set_paint_kit( weapon, skin_cfg ) )
        continue;

      if( g_cfg.misc.menu )
        call_force_update = true;
    }

    if( !call_force_update || interfaces::client_state->delta_tick == -1 || std::abs( interfaces::global_vars->cur_time - last_update_time ) < 1.f )
    {
      if( update_hud && !in_force_update )
      {
        for( auto weapon : weapon_list )
        {
          if( !weapon )
            continue;

          force_update_skin( weapon );
        }

        update_hud = false;
      }

      return;
    }

    interfaces::client_state->delta_tick = -1;

    call_force_update = false;
    in_force_update = update_hud = true;

    last_update_time = interfaces::global_vars->cur_time;
  }
}