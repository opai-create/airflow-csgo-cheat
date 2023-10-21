#pragma once

class c_base_combat_weapon;
class c_cs_player_resource;
struct weapon_info_t;

using c_d3d_device = IDirect3DDevice9;

class c_hacks
{
private:
    struct modules_t
    {
        HMODULE serverbrowser{};
        HMODULE datacache{};
        HMODULE client{};
        HMODULE engine{};
        HMODULE materialsystem{};
        HMODULE vstdlib{};
        HMODULE vphysics{};
        HMODULE vgui2{};
        HMODULE server{};
        HMODULE gameoverlayrenderer{};
        HMODULE shaderapidx9{};
        HMODULE tier0{};
        HMODULE localize{};
        HMODULE filesystem_stdio{};
        HMODULE vguimatsurface{};
        HMODULE studiorender{};

#ifdef _DEBUG
        HMODULE dllmain{};
#endif
        void init(bool start = false);
    };

    struct convars_t
    {
        c_convar* mp_teammates_are_enemies{};
        c_convar* weapon_debug_spread_show{};
        c_convar* cl_foot_contact_shadows{};
        c_convar* cl_brushfastpath{};
        c_convar* cl_csm_shadows{};
        c_convar* sv_cheats{};
        c_convar* sv_skyname{};
        c_convar* viewmodel_fov{};
        c_convar* weapon_accuracy_nospread{};
        c_convar* weapon_recoil_scale{};
        c_convar* cl_lagcompensation{};
        c_convar* cl_sidespeed{};
        c_convar* m_pitch{};
        c_convar* m_yaw{};
        c_convar* sensitivity{};
        c_convar* cl_updaterate{};
        c_convar* cl_interp{};
        c_convar* cl_interp_ratio{};
        c_convar* zoom_sensitivity_ratio_mouse{};
        c_convar* mat_fullbright{};
        c_convar* mat_postprocess_enable{};
        c_convar* fog_override{};
        c_convar* fog_start{};
        c_convar* fog_end{};
        c_convar* fog_maxdensity{};
        c_convar* fog_color{};
        c_convar* sv_gravity{};
        c_convar* sv_maxunlag{};
        c_convar* sv_unlag{};
        c_convar* cl_csm_rot_override{};
        c_convar* cl_csm_rot_x{};
        c_convar* cl_csm_rot_y{};
        c_convar* cl_csm_rot_z{};
        c_convar* cl_csm_max_shadow_dist{};
        c_convar* sv_footsteps{};
        c_convar* cl_clock_correction{};
        c_convar* sv_friction{};
        c_convar* sv_stopspeed{};
        c_convar* sv_jump_impulse{};
        c_convar* weapon_molotov_maxdetonateslope{};
        c_convar* molotov_throw_detonate_time{};
        c_convar* mp_damage_scale_ct_head{};
        c_convar* mp_damage_scale_ct_body{};
        c_convar* mp_damage_scale_t_head{};
        c_convar* mp_damage_scale_t_body{};
        c_convar* ff_damage_reduction_bullets{};
        c_convar* ff_damage_bullet_penetration{};
        c_convar* sv_accelerate{};
        c_convar* weapon_accuracy_shotgun_spread_patterns{};
        c_convar* cl_wpn_sway_interp{};
        c_convar* cl_forwardspeed{};
        c_convar* sv_minupdaterate{};
        c_convar* sv_maxupdaterate{};
        c_convar* sv_client_min_interp_ratio{};
        c_convar* sv_client_max_interp_ratio{};
        c_convar* cl_interpolate{};
        c_convar* cl_pred_doresetlatch{};
        c_convar* cl_cmdrate{};
        c_convar* rate{};
        c_convar* r_DrawSpecificStaticProp{};
        c_convar* sv_maxusrcmdprocessticks{};
        c_convar* sv_clip_penetration_traces_to_players{};
        c_convar* sv_accelerate_use_weapon_speed{};
        c_convar* sv_maxvelocity{};
        c_convar* mp_solid_teammates{};
        c_convar* sv_clockcorrection_msecs{};
        c_convar* con_filter_text{};
        c_convar* con_filter_enable{};
        c_convar* sv_airaccelerate{};
        c_convar* sv_enablebunnyhopping{};

        void init();
    };

    struct cheat_info_t
    {
        memory::bits_t eggs = 0;
        std::string user_name{};
        std::string user_avatar{};
        std::string user_token{};
    };

public:
    HWND window;

#ifdef _DEBUG
    bool unload = false;
#endif

    bool fake_datagram{};
    bool cl_lagcomp0{};
    bool loading_config{};
    bool in_game{};
    bool valve_ds{};
    bool cheat_init{};
    bool cheat_init2{};
    bool open_console{};
    bool can_peek{};
    bool shooting{};

    bool* send_packet{};

    int tickbase{};
    int max_choke{};
    int tick_rate{};
    int shot_cmd{};
    int arrival_tick{};

    float outgoing{};
    float real_ping{};
    float ping{};
    float max_unlag{};
    float lerp_time{};
    float original_frame_time{};
    float predicted_time{};
    float ideal_inaccuracy{};

    std::vector<int> last_outgoing_commands{};
    std::string exe_path{};

    cheat_info_t cheat_info{};

    c_client* client{};
    c_client_mode* client_mode{};
    c_client_state* client_state{};
    c_engine* engine{};
    c_entity_list* entity_list{};
    c_panel* panel{};
    c_model_render* model_render{};
    c_debug_overlay* debug_overlay{};
    c_material_system* material_system{};
    c_game_movement* game_movement{};
    c_prediction* prediction{};
    c_move_helper* move_helper{};
    c_engine_cvar* cvar{};
    c_model_info* model_info{};
    c_input* input{};
    c_game_event_manager2* game_event_manager{};
    c_phys_surface_props* phys_surface_props{};
    c_render_view* render_view{};
    c_view_render* view_render{};
    c_glow_object_manager* glow_object_manager{};
    c_global_vars* global_vars{};
    c_weapon_system* weapon_system{};
    c_engine_trace* engine_trace{};
    c_d3d_device* d3d_device{};
    c_key_values_system* key_values_system{};
    c_network_string_table_container* network_string_table_container{};
    c_item_schema* item_schema{};
    c_localize* localize{};
    c_game_rules* game_rules{};
    c_view_render_beams* view_render_beams{};
    void* file_system{};
    c_surface* surface{};
    c_static_prop_manager* static_prop_manager{};
    c_studio_render* studio_render{};
    c_cs_player_resource* player_resource{};
    c_model_cache* model_cache{};
    void* engine_sound{};

    c_cs_player* local{};
    c_base_combat_weapon* weapon{};
    weapon_info_t* weapon_info{};
    c_user_cmd* cmd{};

    modules_t modules{};
    convars_t convars{};

    INLINE void reset_ctx_values() 
    {
        local = nullptr;
        weapon = nullptr;
        weapon_info = nullptr;
        cmd = nullptr;

        cl_lagcomp0 = false;
        in_game = false;
        valve_ds = false;
        can_peek = false;
        shooting = false;

        tickbase = 0;
        max_choke = 0;
        tick_rate = 0;
        shot_cmd = 0;
        arrival_tick = 0;

        real_ping = 0.f;
        outgoing = 0.f;
        ping = 0.f;
        max_unlag = 0.f;
        lerp_time = 0.f;
        original_frame_time = 0.f;
        predicted_time = 0.f;
        ideal_inaccuracy = 0.f;

        last_outgoing_commands.clear();
    }

    INLINE float system_time()
    {
        return (float)(clock() / (float)1000.f);
    }

    void init_local_player();
    void init_interfaces();
    void update_dynamic_interfaces();
    void init_main(LPVOID reserved);
    void init(LPVOID reserved);
    c_hacks();
};

#ifdef _DEBUG
inline auto HACKS = std::make_unique<c_hacks>();
#else
CREATE_DUMMY_PTR(c_hacks);
DECLARE_XORED_PTR(c_hacks, GET_XOR_KEYUI32);

#define HACKS XORED_PTR(c_hacks)
#endif

INLINE void init_cheat(LPVOID reserved)
{
#ifdef _DEBUG
    HACKS->init_main(reserved);
#else
    HACKS->init(reserved);
#endif
}