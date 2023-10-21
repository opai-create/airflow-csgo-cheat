#include "game_functions.h"

#include "../sdk/entity.h"

namespace func_ptrs
{
  get_name_fn get_name{ };
  init_key_values_fn init_key_values{ };
  load_from_buffer_fn load_from_buffer{ };
  load_named_sky_fn load_named_sky{ };
  physics_run_think_fn physics_run_think{ };
  think_fn think{ };
  create_animstate_fn create_animstate{ };
  reset_animstate_fn reset_animstate{ };
  update_animstate_fn update_animstate{ };
  set_abs_angles_fn set_abs_angles{ };
  set_abs_origin_fn set_abs_origin{ };
  get_pose_parameter_fn get_pose_parameter{ };
  update_merge_cache_fn update_merge_cache{ };
  add_dependencies_fn add_dependencies{ };
  attachments_helper_fn attachments_helper{ };
  copy_to_follow_fn copy_to_follow{ };
  copy_from_follow_fn copy_from_follow{ };
  accumulate_pose_fn accumulate_pose{ };
  show_and_update_selection_fn show_and_update_selection{ };
  invalidate_physics_recursive_fn invalidate_physics_recursive{ };
  lookup_bone_fn lookup_bone{ };
  write_user_cmd_fn write_user_cmd{ };
  post_think_physics_fn post_think_physics{ };
  simulate_player_simulated_entities_fn simulate_player_simulated_entities{ };
  set_collision_bounds_fn set_collision_bounds{ };
  find_hud_element_fn find_hud_element{ };
  compute_hitbox_surrounding_box_fn compute_hitbox_surrounding_box{ };
  is_breakable_entity_fn is_breakable_entity{ };
  calc_shotgun_spread_fn calc_shotgun_spread{ };
  weapon_shootpos_fn weapon_shootpos{ };
  update_all_viewmodel_addons_fn update_all_viewmodel_addons{ };
  get_viewmodel_fn get_viewmodel{ };
  calc_absolute_position_fn calc_absolute_position{ };

  bool* override_processing{ };
  int smoke_count{ };

  __forceinline void init( )
  {
    get_name = patterns::get_name.as< get_name_fn >( );
    init_key_values = patterns::init_key_values.as< init_key_values_fn >( );
    load_from_buffer = patterns::load_from_buffer.as< load_from_buffer_fn >( );
    load_named_sky = patterns::load_named_sky.as< load_named_sky_fn >( );
    physics_run_think = patterns::physics_run_think.as< physics_run_think_fn >( );
    think = patterns::think.as< think_fn >( );
    create_animstate = patterns::create_animstate.as< create_animstate_fn >( );
    reset_animstate = patterns::reset_animstate.as< reset_animstate_fn >( );
    update_animstate = patterns::update_animstate.as< update_animstate_fn >( );
    set_abs_angles = patterns::set_abs_angles.as< set_abs_angles_fn >( );
    set_abs_origin = patterns::set_abs_origin.as< set_abs_origin_fn >( );
    get_pose_parameter = patterns::get_pose_parameter.as< get_pose_parameter_fn >( );
    update_merge_cache = patterns::update_merge_cache.as< update_merge_cache_fn >( );
    add_dependencies = patterns::add_dependencies.as< add_dependencies_fn >( );
    attachments_helper = patterns::attachments_helper.as< attachments_helper_fn >( );
    copy_to_follow = patterns::copy_to_follow.as< copy_to_follow_fn >( );
    copy_from_follow = patterns::copy_from_follow.as< copy_from_follow_fn >( );
    accumulate_pose = patterns::accumulate_pose.as< accumulate_pose_fn >( );
    invalidate_physics_recursive = patterns::invalidate_physics_recursive.as< invalidate_physics_recursive_fn >( );
    lookup_bone = patterns::lookup_bone.as< lookup_bone_fn >( );
    write_user_cmd = patterns::write_user_cmd.as< write_user_cmd_fn >( );
    post_think_physics = patterns::post_think_physics.as< post_think_physics_fn >( );
    simulate_player_simulated_entities = patterns::simulate_player_simulated_entities.as< simulate_player_simulated_entities_fn >( );
    set_collision_bounds = patterns::set_collision_bounds.as< set_collision_bounds_fn >( );
    find_hud_element = patterns::find_hud_element.as< find_hud_element_fn >( );
    compute_hitbox_surrounding_box = patterns::compute_hitbox_surrounding_box.as< compute_hitbox_surrounding_box_fn >( );
    is_breakable_entity = patterns::is_breakable_entity.as< is_breakable_entity_fn >( );
    calc_shotgun_spread = patterns::calc_shotgun_spread.as< calc_shotgun_spread_fn >( );
    weapon_shootpos = patterns::weapon_shootpos.as< weapon_shootpos_fn >( );
    show_and_update_selection = patterns::show_and_update_selection.as< show_and_update_selection_fn >( );
    update_all_viewmodel_addons = patterns::update_all_viewmodel_addons.as< update_all_viewmodel_addons_fn >( );
    get_viewmodel = patterns::get_viewmodel.as< get_viewmodel_fn >( );
    calc_absolute_position = patterns::calc_absolute_position.as< calc_absolute_position_fn >( );

    override_processing = *patterns::disable_post_process.as< bool** >( );
    smoke_count = *patterns::remove_smoke.as< int* >( );
  }
}