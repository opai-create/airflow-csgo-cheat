#include "../includes.h"

#include "sdk.h"
#include "global_context.h"

#include "tools/memory/memory.h"

#include "sdk/entity.h"
#include "sdk/c_csplayerresource.h"

namespace interfaces
{
  c_base_client_dll* client{ };
  c_global_vars_base* global_vars{ };
  c_surface* surface{ };
  c_panel* panel{ };
  c_engine* engine{ };
  c_entity_list* entity_list{ };
  c_debug_overlay* debug_overlay{ };
  c_input_system* input_system{ };
  c_input* input{ };
  c_engine_trace* engine_trace{ };
  c_model_render* model_render{ };
  c_material_system* material_system{ };
  c_studio_render* studio_render{ };
  c_localize* localize{ };
  c_convar* convar{ };
  c_prediction* prediction{ };
  c_client_mode* client_mode{ };
  c_movehelper* move_helper{ };
  c_game_movement* game_movement{ };
  c_game_event_manager2* game_event_manager{ };
  c_view_render_beams* beam{ };
  c_player_resource* player_resource{ };
  c_memory_allocate* memory{ };
  c_clientstate* client_state{ };
  c_model_info* model_info{ };
  c_phys_surface_props* phys_surface_props{ };
  c_view_render* view_render{ };
  c_glow_object_manager* glow_object_manager{ };
  c_engine_sound* engine_sound{ };
  c_mdl_cache* model_cache{ };
  c_render_view* render_view{ };
  c_game_rules* game_rules{ };
  c_key_values_system* key_values_system{ };
  c_network_string_table_container* network_string_table_container{ };
  c_item_schema* item_schema{ };
  c_static_prop_manager* static_prop_manager{ };

  bool should_change = true;
  __forceinline void init_dynamic_interfaces( )
  {
    if( !g_ctx.in_game )
    {
      if( !should_change )
        should_change = true;
      return;
    }

    if( !should_change )
      return;

    player_resource = **patterns::player_resource.as< c_player_resource*** >( );
    game_rules = **patterns::game_rules.as< c_game_rules*** >( );

    should_change = false;
  }

  __forceinline void init( )
  {
    client = g_memory->get_interface( modules::client, xor_str_s( "VClient018" ) ).as< c_base_client_dll* >( );
    surface = g_memory->get_interface( modules::vguimatsurface, xor_str_s( "VGUI_Surface031" ) ).as< c_surface* >( );
    panel = g_memory->get_interface( modules::vgui2, xor_str_s( "VGUI_Panel009" ) ).as< c_panel* >( );
    engine = g_memory->get_interface( modules::engine, xor_str_s( "VEngineClient014" ) ).as< c_engine* >( );
    entity_list = g_memory->get_interface( modules::client, xor_str_s( "VClientEntityList003" ) ).as< c_entity_list* >( );
    debug_overlay = g_memory->get_interface( modules::engine, xor_str_s( "VDebugOverlay004" ) ).as< c_debug_overlay* >( );
    input_system = g_memory->get_interface( modules::inputsystem, xor_str_s( "InputSystemVersion001" ) ).as< c_input_system* >( );
    engine_trace = g_memory->get_interface( modules::engine, xor_str_s( "EngineTraceClient004" ) ).as< c_engine_trace* >( );
    model_render = g_memory->get_interface( modules::engine, xor_str_s( "VEngineModel016" ) ).as< c_model_render* >( );
    material_system = g_memory->get_interface( modules::materialsystem, xor_str_s( "VMaterialSystem080" ) ).as< c_material_system* >( );
    studio_render = g_memory->get_interface( modules::studiorender, xor_str_s( "VStudioRender026" ) ).as< c_studio_render* >( );
    localize = g_memory->get_interface( modules::localize, xor_str_s( "Localize_001" ) ).as< c_localize* >( );
    convar = g_memory->get_interface( modules::vstdlib, xor_str_s( "VEngineCvar007" ) ).as< c_convar* >( );
    game_movement = g_memory->get_interface( modules::client, xor_str_s( "GameMovement001" ) ).as< c_game_movement* >( );
    prediction = g_memory->get_interface( modules::client, xor_str_s( "VClientPrediction001" ) ).as< c_prediction* >( );
    game_event_manager = g_memory->get_interface( modules::engine, xor_str_s( "GAMEEVENTSMANAGER002" ) ).as< c_game_event_manager2* >( );
    model_info = g_memory->get_interface( modules::engine, xor_str_s( "VModelInfoClient004" ) ).as< c_model_info* >( );
    phys_surface_props = g_memory->get_interface( modules::vphysics, xor_str_s( "VPhysicsSurfaceProps001" ) ).as< c_phys_surface_props* >( );
    engine_sound = g_memory->get_interface( modules::engine, xor_str_s( "IEngineSoundClient003" ) ).as< c_engine_sound* >( );
    model_cache = g_memory->get_interface( modules::datacache, xor_str_s( "MDLCache004" ) ).as< c_mdl_cache* >( );
    render_view = g_memory->get_interface( modules::engine, xor_str_s( "VEngineRenderView014" ) ).as< c_render_view* >( );
    network_string_table_container = g_memory->get_interface( modules::engine, xor_str_s( "VEngineClientStringTable001" ) ).as< c_network_string_table_container* >( );
    static_prop_manager = g_memory->get_interface( modules::engine, xor_str_s( "StaticPropMgrClient005" ) ).as< c_static_prop_manager* >( );

    input = *patterns::input.as< c_input** >( );
    beam = *patterns::beam.as< c_view_render_beams** >( );
    view_render = **patterns::view_render.as< c_view_render*** >( );
    glow_object_manager = patterns::glow_object.as< c_glow_object_manager*( __cdecl* )( ) >( )( );
    client_state = **patterns::client_state.add( 1 ).as< c_clientstate*** >( );
    move_helper = **patterns::move_helper.as< c_movehelper*** >( );
    item_schema = ( c_item_schema* )( patterns::item_system.as< std::uintptr_t( __cdecl* )( ) >( )( ) + 0x4 );

    memory = *( c_memory_allocate** )( GetProcAddress( modules::tier0, xor_c_s( "g_pMemAlloc" ) ) );
    global_vars = **patterns::global_vars.as< c_global_vars_base*** >( );
    client_mode = **( c_client_mode*** )( g_memory->getvfunc< DWORD >( client, 10 ) + 0x5 );

    key_values_system = key_values_system_fn( GetProcAddress( modules::vstdlib, xor_c_s( "KeyValuesSystem" ) ) )( );
  }
}

namespace cvars
{
  c_cvar* weapon_debug_spread_show{ };
  c_cvar* cl_foot_contact_shadows{ };
  c_cvar* cl_brushfastpath{ };
  c_cvar* cl_csm_shadows{ };
  c_cvar* sv_cheats{ };
  c_cvar* sv_skyname{ };
  c_cvar* viewmodel_fov{ };
  c_cvar* weapon_accuracy_nospread{ };
  c_cvar* weapon_recoil_scale{ };
  c_cvar* cl_lagcompensation{ };
  c_cvar* cl_sidespeed{ };
  c_cvar* m_pitch{ };
  c_cvar* m_yaw{ };
  c_cvar* sensitivity{ };
  c_cvar* cl_updaterate{ };
  c_cvar* cl_interp{ };
  c_cvar* cl_interp_ratio{ };
  c_cvar* zoom_sensitivity_ratio_mouse{ };
  c_cvar* mat_fullbright{ };
  c_cvar* mat_postprocess_enable{ };
  c_cvar* fog_override{ };
  c_cvar* fog_start{ };
  c_cvar* fog_end{ };
  c_cvar* fog_maxdensity{ };
  c_cvar* fog_color{ };
  c_cvar* sv_gravity{ };
  c_cvar* sv_maxunlag{ };
  c_cvar* cl_csm_rot_override{ };
  c_cvar* cl_csm_rot_x{ };
  c_cvar* cl_csm_rot_y{ };
  c_cvar* cl_csm_rot_z{ };
  c_cvar* sv_footsteps{ };
  c_cvar* cl_clock_correction{ };
  c_cvar* sv_friction{ };
  c_cvar* sv_stopspeed{ };
  c_cvar* sv_jump_impulse{ };
  c_cvar* sv_accelerate{ };
  c_cvar* weapon_molotov_maxdetonateslope{ };
  c_cvar* molotov_throw_detonate_time{ };
  c_cvar* mp_damage_scale_ct_head{ };
  c_cvar* mp_damage_scale_ct_body{ };
  c_cvar* mp_damage_scale_t_head{ };
  c_cvar* mp_damage_scale_t_body{ };
  c_cvar* ff_damage_reduction_bullets{ };
  c_cvar* ff_damage_bullet_penetration{ };
  c_cvar* net_showfragments{ };
  c_cvar* cl_wpn_sway_interp{ };
  c_cvar* weapon_accuracy_shotgun_spread_patterns{ };
  c_cvar* cl_forwardspeed{ };
  c_cvar* sv_minupdaterate{ };
  c_cvar* sv_maxupdaterate{ };
  c_cvar* sv_client_min_interp_ratio{ };
  c_cvar* sv_client_max_interp_ratio{ };
  c_cvar* cl_interpolate{ };
  c_cvar* r_DrawSpecificStaticProp{ };

  __forceinline void init( )
  {
    weapon_debug_spread_show = interfaces::convar->find_convar( xor_c_s( "weapon_debug_spread_show" ) );
    cl_foot_contact_shadows = interfaces::convar->find_convar( xor_c_s( "cl_foot_contact_shadows" ) );
    cl_csm_shadows = interfaces::convar->find_convar( xor_c_s( "cl_csm_shadows" ) );
    cl_brushfastpath = interfaces::convar->find_convar( xor_c_s( "cl_brushfastpath" ) );
    sv_cheats = interfaces::convar->find_convar( xor_c_s( "sv_cheats" ) );
    sv_skyname = interfaces::convar->find_convar( xor_c_s( "sv_skyname" ) );
    viewmodel_fov = interfaces::convar->find_convar( xor_c_s( "viewmodel_fov" ) );
    weapon_accuracy_nospread = interfaces::convar->find_convar( xor_c_s( "weapon_accuracy_nospread" ) );
    weapon_recoil_scale = interfaces::convar->find_convar( xor_c_s( "weapon_recoil_scale" ) );
    cl_lagcompensation = interfaces::convar->find_convar( xor_c_s( "cl_lagcompensation" ) );
    cl_sidespeed = interfaces::convar->find_convar( xor_c_s( "cl_sidespeed" ) );
    m_pitch = interfaces::convar->find_convar( xor_c_s( "m_pitch" ) );
    m_yaw = interfaces::convar->find_convar( xor_c_s( "m_yaw" ) );
    sensitivity = interfaces::convar->find_convar( xor_c_s( "sensitivity" ) );
    cl_updaterate = interfaces::convar->find_convar( xor_c_s( "cl_updaterate" ) );
    cl_interp = interfaces::convar->find_convar( xor_c_s( "cl_interp" ) );
    cl_interp_ratio = interfaces::convar->find_convar( xor_c_s( "cl_interp_ratio" ) );
    zoom_sensitivity_ratio_mouse = interfaces::convar->find_convar( xor_c_s( "zoom_sensitivity_ratio_mouse" ) );
    mat_fullbright = interfaces::convar->find_convar( xor_c_s( "mat_fullbright" ) );
    mat_postprocess_enable = interfaces::convar->find_convar( xor_c_s( "mat_postprocess_enable" ) );
    fog_override = interfaces::convar->find_convar( xor_c_s( "fog_override" ) );
    fog_start = interfaces::convar->find_convar( xor_c_s( "fog_start" ) );
    fog_end = interfaces::convar->find_convar( xor_c_s( "fog_end" ) );
    fog_maxdensity = interfaces::convar->find_convar( xor_c_s( "fog_maxdensity" ) );
    fog_color = interfaces::convar->find_convar( xor_c_s( "fog_color" ) );
    sv_gravity = interfaces::convar->find_convar( xor_c_s( "sv_gravity" ) );
    sv_maxunlag = interfaces::convar->find_convar( xor_c_s( "sv_maxunlag" ) );
    cl_csm_rot_override = interfaces::convar->find_convar( xor_c_s( "cl_csm_rot_override" ) );
    cl_csm_rot_x = interfaces::convar->find_convar( xor_c_s( "cl_csm_rot_x" ) );
    cl_csm_rot_y = interfaces::convar->find_convar( xor_c_s( "cl_csm_rot_y" ) );
    cl_csm_rot_z = interfaces::convar->find_convar( xor_c_s( "cl_csm_rot_z" ) );
    sv_footsteps = interfaces::convar->find_convar( xor_c_s( "sv_footsteps" ) );
    cl_clock_correction = interfaces::convar->find_convar( xor_c_s( "cl_clock_correction" ) );
    sv_friction = interfaces::convar->find_convar( xor_c_s( "sv_friction" ) );
    sv_stopspeed = interfaces::convar->find_convar( xor_c_s( "sv_stopspeed" ) );
    sv_jump_impulse = interfaces::convar->find_convar( xor_c_s( "sv_jump_impulse" ) );
    sv_accelerate = interfaces::convar->find_convar( xor_c_s( "sv_accelerate" ) );
    weapon_molotov_maxdetonateslope = interfaces::convar->find_convar( xor_c_s( "weapon_molotov_maxdetonateslope" ) );
    molotov_throw_detonate_time = interfaces::convar->find_convar( xor_c_s( "molotov_throw_detonate_time" ) );
    mp_damage_scale_ct_head = interfaces::convar->find_convar( xor_c_s( "mp_damage_scale_ct_head" ) );
    mp_damage_scale_ct_body = interfaces::convar->find_convar( xor_c_s( "mp_damage_scale_ct_body" ) );
    mp_damage_scale_t_head = interfaces::convar->find_convar( xor_c_s( "mp_damage_scale_t_head" ) );
    mp_damage_scale_t_body = interfaces::convar->find_convar( xor_c_s( "mp_damage_scale_t_body" ) );
    ff_damage_reduction_bullets = interfaces::convar->find_convar( xor_c_s( "ff_damage_reduction_bullets" ) );
    ff_damage_bullet_penetration = interfaces::convar->find_convar( xor_c_s( "ff_damage_bullet_penetration" ) );
    net_showfragments = interfaces::convar->find_convar( xor_c_s( "net_showfragments" ) );
    cl_wpn_sway_interp = interfaces::convar->find_convar( xor_c_s( "cl_wpn_sway_interp" ) );
    weapon_accuracy_shotgun_spread_patterns = interfaces::convar->find_convar( xor_c_s( "weapon_accuracy_shotgun_spread_patterns" ) );
    cl_forwardspeed = interfaces::convar->find_convar( xor_c_s( "cl_forwardspeed" ) );

    sv_minupdaterate = interfaces::convar->find_convar( xor_c_s( "sv_minupdaterate" ) );
    sv_maxupdaterate = interfaces::convar->find_convar( xor_c_s( "sv_maxupdaterate" ) );
    sv_client_min_interp_ratio = interfaces::convar->find_convar( xor_c_s( "sv_client_min_interp_ratio" ) );
    sv_client_max_interp_ratio = interfaces::convar->find_convar( xor_c_s( "sv_client_max_interp_ratio" ) );
    cl_interpolate = interfaces::convar->find_convar( xor_c_s( "sv_client_max_interp_ratio" ) );
    r_DrawSpecificStaticProp = interfaces::convar->find_convar( xor_c_s( "r_DrawSpecificStaticProp" ) );
  }
}

// thx ekzi XD
void c_global_context::update_animations( )
{
  if( last_time_updated == -1.f )
    last_time_updated = this->system_time( );

  animation_speed = std::fabsf( last_time_updated - this->system_time( ) ) * 5.f;
  last_time_updated = this->system_time( );
}

void c_global_context::update_local_player( )
{
  local = **patterns::local.as< c_csplayer*** >( );

  if( !local || !local->is_alive( ) )
  {
    weapon = nullptr;
    weapon_info = nullptr;
    return;
  }

  weapon = local->get_active_weapon( );
  if( weapon )
    weapon_info = weapon->get_weapon_info( );
}