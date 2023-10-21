#pragma once
#include "../tools/memory/memory.h"

enum
{
  char_tex_antlion = 'A',
  char_tex_bloodyflesh = 'B',
  char_tex_concrete = 'C',
  char_tex_dirt = 'D',
  char_tex_eggshell = 'E',
  char_tex_flesh = 'F',
  char_tex_grate = 'G',
  char_tex_alienflesh = 'H',
  char_tex_clip = 'I',
  char_tex_plastic = 'L',
  char_tex_metal = 'M',
  char_tex_sand = 'N',
  char_tex_foliage = 'O',
  char_tex_computer = 'P',
  char_tex_slosh = 'S',
  char_tex_tile = 'T',
  char_tex_cardboard = 'U',
  char_tex_vent = 'V',
  char_tex_wood = 'W',
  char_tex_glass = 'Y',
  char_tex_warpshield = 'Z',
};

struct surfacephysicsparams_t
{
  float Friction;
  float Elasticity;
  float Density;
  float Thickness;
  float Dampening;
};

struct surfaceaudioparams_t
{
  float Reflectivity;          // like elasticity, but how much sound should be reflected by this surface
  float HardnessFactor;        // like elasticity, but only affects impact sound choices
  float RoughnessFactor;       // like friction, but only affects scrape sound choices
  float RoughThreshold;        // surface roughness > this causes "rough" scrapes, < this causes "smooth" scrapes
  float HardThreshold;         // surface hardness > this causes "hard" impacts, < this causes "soft" impacts
  float HardVelocityThreshold; // collision velocity > this causes "hard" impacts, < this causes "soft" impacts
  float HighPitchOcclusion;    // a value betweeen 0 and 100 where 0 is not occluded at all and 100 is silent (except for any additional reflected sound)
  float MidPitchOcclusion;
  float LowPitchOcclusion;
};

struct surfacesoundnames_t
{
  std::uint16_t WalkStepLeft;
  std::uint16_t WalkStepRight;
  std::uint16_t RunStepLeft;
  std::uint16_t RunStepRight;
  std::uint16_t ImpactSoft;
  std::uint16_t ImpactHard;
  std::uint16_t ScrapeSmooth;
  std::uint16_t ScrapeRough;
  std::uint16_t BulletImpact;
  std::uint16_t Rolling;
  std::uint16_t BreakSound;
  std::uint16_t StrainSound;
};

struct surfacesoundhandles_t
{
  std::uint16_t WalkStepLeft;
  std::uint16_t WalkStepRight;
  std::uint16_t RunStepLeft;
  std::uint16_t RunStepRight;
  std::uint16_t ImpactSoft;
  std::uint16_t ImpactHard;
  std::uint16_t ScrapeSmooth;
  std::uint16_t ScrapeRough;
  std::uint16_t BulletImpact;
  std::uint16_t Rolling;
  std::uint16_t BreakSound;
  std::uint16_t StrainSound;
};

struct surfacegameprops_t
{
  float max_speed_facotr;
  float jump_factor;
  float penetration_modifier;
  float damage_modifier;
  std::uint16_t material;
  std::byte climbable;
  std::byte pad0 [ 0x4 ];
};

struct surfacedata_t
{
  surfacephysicsparams_t physics;
  surfaceaudioparams_t audio;
  surfacesoundnames_t sounds;
  surfacegameprops_t game;
  surfacesoundhandles_t soundhandles;
};

class c_phys_surface_props
{
public:
  surfacedata_t* get_surface_data( int surface_index )
  {
    using fn = surfacedata_t*( __thiscall* )( void*, int );
    return g_memory->getvfunc< fn >( this, 5 )( this, surface_index );
  }
};