#include "protect.h"

namespace xor_strs
{
  std::string hitbox_head{ };
  std::string hitbox_chest{ };
  std::string hitbox_stomach{ };
  std::string hitbox_pelvis{ };
  std::string hitbox_arms{ };
  std::string hitbox_legs{ };
  std::string hitbox_body{ };
  std::string hitbox_limbs{ };

  std::string weapon_default{ };
  std::string weapon_auto{ };
  std::string weapon_heavy_pistols{ };
  std::string weapon_pistols{ };
  std::string weapon_ssg08{ };
  std::string weapon_awp{ };
  std::string weapon_negev{ };
  std::string weapon_m249{ };
  std::string weapon_ak47{ };
  std::string weapon_aug{ };
  std::string weapon_duals{ };
  std::string weapon_p250{ };
  std::string weapon_cz{ };

  std::string aa_disabled{ };
  std::string aa_default{ };

  std::string aa_pitch_down{ };
  std::string aa_pitch_up{ };
  std::string aa_pitch_minimal{ };

  std::string aa_stand;
  std::string aa_move;
  std::string aa_air;

  std::string aa_yaw_back{ };
  std::string aa_yaw_spin{ };
  std::string aa_yaw_crooked{ };

  std::string aa_jitter_center{ };
  std::string aa_jitter_offset{ };
  std::string aa_jitter_random{ };

  std::string aa_desync_jitter{ };

  std::string aa_fakelag_max{ };
  std::string aa_fakelag_jitter{ };

  std::string vis_chams_textured{ };
  std::string vis_chams_metallic{ };
  std::string vis_chams_flat{ };
  std::string vis_chams_glass{ };
  std::string vis_chams_glow{ };
  std::string vis_chams_bubble{ };
  std::string vis_chams_money{ };
  std::string vis_chams_fadeup{ };

  std::string buybot_none{ };

  std::string cfg_main{ };
  std::string cfg_additional{ };
  std::string cfg_misc{ };
  std::string cfg_custom1{ };
  std::string cfg_custom2{ };

  std::string chams_visible{ };
  std::string chams_xqz{ };
  std::string chams_history{ };
  std::string chams_onshot{ };
  std::string chams_ragdolls{ };
  std::string chams_viewmodel{ };
  std::string chams_wpn{ };
  std::string chams_attachments{ };
  std::string chams_fake{ };
  std::string chams_fakelag{ };

  std::string sound_metallic{ };
  std::string sound_tap{ };

  std::string box_default{ };
  std::string box_thin{ };

  std::string ragdoll_away{ };
  std::string ragdoll_fly{ };

  std::string target_damage{ };
  std::string target_distance{ };

  std::string tracer_beam{ };
  std::string tracer_line{ };

  __forceinline void init( )
  {
    hitbox_head = xor_str( "Head" );
    hitbox_chest = xor_str( "Chest" );
    hitbox_stomach = xor_str( "Stomach" );
    hitbox_pelvis = xor_str( "Pelvis" );
    hitbox_arms = xor_str( "Arms" );
    hitbox_legs = xor_str( "Legs" );
    hitbox_body = xor_str( "Body" );
    hitbox_limbs = xor_str( "Limbs" );

    weapon_default = xor_str( "General" );
    weapon_auto = xor_str( "Auto snipers" );
    weapon_heavy_pistols = xor_str( "Deagle/R8" );
    weapon_pistols = xor_str( "Pistols" );
    weapon_ssg08 = xor_str( "Scout" );
    weapon_awp = xor_str( "AWP" );
    weapon_negev = xor_str( "Negev" );
    weapon_m249 = xor_str( "M249" );
    weapon_ak47 = xor_str( "AK47/M4A1" );
    weapon_aug = xor_str( "AUG/SG553" );
    weapon_duals = xor_str( "Dual berettas" );
    weapon_p250 = xor_str( "P250" );
    weapon_cz = xor_str( "CZ75/Five seven" );

    aa_disabled = xor_str( "Disabled" );
    aa_default = xor_str( "Default" );

    aa_pitch_down = xor_str( "Down" );
    aa_pitch_up = xor_str( "Up" );
    aa_pitch_minimal = xor_str( "Minimal" );

    aa_stand = xor_str( "Stand" );
    aa_move = xor_str( "Move" );
    aa_air = xor_str( "Air" );

    aa_yaw_back = xor_str( "Backward" );
    aa_yaw_spin = xor_str( "Distortion" );
    aa_yaw_crooked = xor_str( "Crooked" );

    aa_jitter_center = xor_str( "Center" );
    aa_jitter_offset = xor_str( "Offset" );
    aa_jitter_random = xor_str( "Random" );

    aa_desync_jitter = xor_str( "Flip" );

    aa_fakelag_max = xor_str( "Maximum" );
    aa_fakelag_jitter = xor_str( "Jitter" );

    vis_chams_textured = xor_str( "Textured" );
    vis_chams_metallic = xor_str( "Shaded" );
    vis_chams_flat = xor_str( "Flat" );
    vis_chams_glass = xor_str( "Metallic" );
    vis_chams_glow = xor_str( "Glow" );
    vis_chams_bubble = xor_str( "Bubble" );
    vis_chams_money = xor_str( "Custom Sprite" );
    vis_chams_fadeup = xor_str( "Fade Up" );

    buybot_none = xor_str( "None" );

    cfg_main = xor_str( "Main" );
    cfg_additional = xor_str( "Additional" );
    cfg_misc = xor_str( "Misc" );
    cfg_custom1 = xor_str( "Custom1" );
    cfg_custom2 = xor_str( "Custom2" );

    chams_visible = xor_str( "Visible" );
    chams_xqz = xor_str( "Through walls" );
    chams_history = xor_str( "History" );
    chams_onshot = xor_str( "On-shot" );
    chams_ragdolls = xor_str( "Ragdolls" );
    chams_viewmodel = xor_str( "Viewmodel" );
    chams_wpn = xor_str( "Weapon" );
    chams_attachments = xor_str( "Attachments" );
    chams_fake = xor_str( "Fake" );
    chams_fakelag = xor_str( "Fake lag" );

    sound_metallic = xor_str( "Metallic" );
    sound_tap = xor_str( "Custom" );

    box_default = xor_str( "Default" );
    box_thin = xor_str( "Thin" );

    ragdoll_away = xor_str( "Away" );
    ragdoll_fly = xor_str( "Fly up" );

    target_distance = xor_str( "Lowest distance" );
    target_damage = xor_str( "Highest damage" );

    tracer_beam = xor_str( "Beam" );
    tracer_line = xor_str( "Line" );
  }
}