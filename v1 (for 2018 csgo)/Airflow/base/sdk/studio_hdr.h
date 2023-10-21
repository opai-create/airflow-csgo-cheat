#pragma once
#include "../tools/math.h"
#include "../tools/netvar_parser.h"

#include "c_utlvector.h"
#include "c_material.h"

enum mod_types_t
{
  mod_bad = 0,
  mod_brush = 1,
  mod_sprite = 2,
  mod_studio = 3,
};

enum damage_type_t
{
  damage_no = 0,
  damage_events_only = 1,
  damage_yes = 2,
  damage_aim = 3,
};

enum hitboxes_t
{
  hitbox_head = 0,
  hitbox_neck,
  hitbox_lower_neck,
  hitbox_pelvis,
  hitbox_stomach,
  hitbox_lower_chest,
  hitbox_chest,
  hitbox_upper_chest,
  hitbox_left_thigh,
  hitbox_right_thigh,
  hitbox_left_calf,
  hitbox_right_calf,
  hitbox_left_foot,
  hitbox_right_foot,
  hitbox_left_hand,
  hitbox_right_hand,
  hitbox_left_upper_arm,
  hitbox_left_forearm,
  hitbox_right_upper_arm,
  hitbox_right_forearm,
  hitbox_max
};

enum hitgroup_t
{
  hitgroup_generic = 0,
  hitgroup_head = 1,
  hitgroup_chest = 2,
  hitgroup_stomach = 3,
  hitgroup_leftarm = 4,
  hitgroup_rightarm = 5,
  hitgroup_leftleg = 6,
  hitgroup_rightleg = 7,
  hitgroup_neck = 8,
  hitgroup_gear = 10
};

struct studio_lod_data_t
{
  void* mesh_data{ };
  float switch_point{ };
  int num_materials{ };
  c_material** material_pointers{ };
  int* material_flags{ };
  int* morph_decal_bone_map{ };
  int decal_bone_count{ };
};

struct studio_hw_data_t
{
  int root_lods{ };
  int num_lods{ };

  studio_lod_data_t* lods{ };

  int studio_meshes{ };

  inline float get_lod_metrics( float sphere_size ) const
  {
    return ( sphere_size != 0.f ) ? ( 100.f / sphere_size ) : 0.f;
  }

  inline int get_lod_for_metric( float lod_metric ) const
  {
    if( !num_lods )
      return 0;

    int num_lods = ( lods [ num_lods - 1 ].switch_point < 0.0f ) ? num_lods - 1 : num_lods;

    for( int i = root_lods; i < num_lods - 1; i++ )
    {
      if( lods [ i + 1 ].switch_point > lod_metric )
        return i;
    }

    return num_lods - 1;
  }
};

struct mstudio_bone_t
{
  int name_index{ };

  inline char* const get_name( void ) const
  {
    return ( ( char* )this ) + name_index;
  }

  int parent{ };
  int bone_controller [ 6 ]{ };

  vector3d pos{ };
  vector4d quat{ };

  float rot [ 3 ]{ };

  vector3d pos_scale{ };
  vector3d rot_scale{ };

  matrix3x4_t pose_to_bone{ };

  quaternion alignment{ };

  int flags{ };

  int proc_type{ };
  int proc_index{ };

  mutable int physics_bone{ };

  inline void* get_procedure( ) const
  {
    if( proc_index == 0 )
      return NULL;
    else
      return ( void* )( ( ( unsigned char* )this ) + proc_index );
  };

  int surface_prop_iindex;

  inline char* const get_surface_prop_name( void ) const
  {
    return ( ( char* )this ) + surface_prop_iindex;
  }

  inline int get_surface_prop( void ) const
  {
    return surface_prop_lookup;
  }

  int contents{ };
  int surface_prop_lookup{ };
  int unused [ 7 ]{ };
};

struct mstudio_bbox_t
{
  int bone{ };
  int group{ };

  vector3d bbmin{ };
  vector3d bbmax{ };

  int hitbox_name_index{ };

  vector3d rotation{ };

  float radius{ };

  int pad2 [ 4 ]{ };
};

struct mstudio_pose_param_desc_t
{
  int name_index{ };

  inline char* const get_name( void ) const
  {
    return ( ( char* )this ) + name_index;
  }

  int flags{ };

  float start{ };
  float end{ };
  float loop{ };
};

struct mstudio_hitbox_set_t
{
  int index_name{ };

  int hitboxes{ };
  int hitbox_index{ };

  inline char* const get_name( void ) const
  {
    return ( ( char* )this ) + index_name;
  }

  inline mstudio_bbox_t* get_hitbox( int i ) const
  {
    return ( mstudio_bbox_t* )( ( ( unsigned char* )this ) + hitbox_index ) + i;
  }
};

struct studio_hdr_t
{
  int id{ };
  int version{ };

  long checksum{ };

  char name [ 64 ]{ };

  int length{ };

  vector3d eye_pos{ };
  vector3d ilum_pos{ };

  vector3d hull_min{ };
  vector3d hull_max{ };

  vector3d bb_min{ };
  vector3d bb_max{ };

  int flags{ };

  int bones{ };
  int bone_index{ };

  int bone_controllers{ };
  int bone_controller_index{ };

  int hitbox_sets{ };
  int hitbox_set_index{ };

  int local_anim{ };
  int local_anim_index{ };

  int local_seq{ };
  int local_seq_index{ };

  int activity_list_version{ };
  int events_indexed{ };

  int textures{ };
  int texture_index{ };

  int numskinref{ };
  int numskinfamilies{ };
  int skinindex{ };

  int numbodyparts{ };
  int bodypartindex{ };

  int numlocalattachments{ };
  int localattachmentindex{ };

  int numlocalnodes{ };
  int localnodeindex{ };
  int localnodenameindex{ };

  int numflexdesc{ };
  int flexdescindex{ };

  inline const char* get_name( void ) const
  {
    return name;
  }

  mstudio_hitbox_set_t* get_hitbox_set( int i )
  {
    if( i > hitbox_sets )
      return nullptr;

    return ( mstudio_hitbox_set_t* )( ( uint8_t* )this + hitbox_set_index ) + i;
  }

  mstudio_bone_t* get_bone( int i )
  {
    if( i > bones )
      return nullptr;

    return ( mstudio_bone_t* )( ( uint8_t* )this + bone_index ) + i;
  }
};

class c_studiohdr
{
public:
  netvar_ref( bone_flags, c_utlvector< int >, 0x30 )
};