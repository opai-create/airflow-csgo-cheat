#pragma once
#include "../config_vars.h"
#include "../../base/hooks/hooks.h"

#include "../../base/interfaces/studio_render.h"
#include "../../base/interfaces/model_render.h"

class c_csplayer;
class c_game_event;

class c_chams
{
private:
  chams_t safe_left_chams = { true, true, 3, 100, 5, c_float_color( 255, 0, 0, 100 ), c_float_color( 255, 0, 0, 100 ) };

  chams_t safe_right_chams = {
    true,
    true,
    3,
    100,
    5,
    c_float_color( 0, 255, 0, 100 ),
    c_float_color( 0, 255, 0, 100 ),
  };

  chams_t safe_zero_chams = {
    true,
    true,
    3,
    100,
    5,
    c_float_color( 0, 0, 255, 100 ),
    c_float_color( 0, 0, 255, 100 ),
  };

  struct hook_data_t
  {
    draw_model_execute_fn original{ };
    void* ecx{ };
    void* ctx{ };
    draw_model_state_t state{ };
    model_render_info_t info{ };
    matrix3x4_t* bone_to_world{ };

    __forceinline void init( draw_model_execute_fn original, void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world )
    {
      this->original = original;
      this->ecx = ecx;
      this->ctx = ctx;
      this->state = state;
      this->info = info;
      this->bone_to_world = bone_to_world;
    }

    __forceinline void call_original( matrix3x4_t* matrix = nullptr )
    {
      original( ecx, ctx, state, info, matrix == nullptr ? bone_to_world : matrix );
    }
  };

  struct shot_record_t
  {
    float time{ };
    float alpha = 1.f;

    model_render_info_t info{ };
    draw_model_state_t state{ };

    vector3d origin{ };

    alignas( 16 ) matrix3x4_t bones [ 128 ]{ };
    alignas( 16 ) matrix3x4_t current_bones{ };
  };

  bool should_init{ };
  bool in_do_post_screen_effects{ };
  std::array< c_material*, 8 > materials_list{ };

  hook_data_t hook_data{ };

  std::vector< shot_record_t > shot_records{ };

  bool is_valid( )
  {
    return materials_list.data( ) != nullptr;
  }

  bool draw_model( chams_t& chams, matrix3x4_t* matrix = nullptr, float alpha = 1.f, bool xqz = false );
  bool get_backtrack_matrix( matrix3x4_t* bones, c_csplayer* player, float& alpha );
  bool should_draw( );

public:
  c_material* custom_world{ };

  void init_materials( );
  void on_draw_model_execute( draw_model_execute_fn original, void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world );

  void add_shot_record( c_csplayer* player, matrix3x4_t* matrix );
  void on_post_screen_effects( );
};