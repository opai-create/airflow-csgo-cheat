#pragma once
#include "../../base/tools/math.h"

#include "../../base/sdk/studio_hdr.h"
#include "../../base/sdk/entity.h"

namespace bone_merge
{
  uintptr_t& get_bone_merge( c_csplayer* player );
  void update_cache( uintptr_t bonemerge );
}

class c_bone_builder
{
private:
  void get_skeleton( vector3d* position, quaternion* q );
  void studio_build_matrices( c_studiohdr* hdr, const matrix3x4_t& world_transform, vector3d* pos, quaternion* q, int mask, matrix3x4_t* out, uint32_t* bone_computed );

public:
  bool filled{ };

  bool ik_ctx{ };
  bool attachments{ };
  bool dispatch{ };

  int mask{ };
  int layer_count{ };

  float time{ };

  matrix3x4_t* matrix{ };
  c_studiohdr* hdr{ };
  c_animation_layers* layers{ };
  c_csplayer* animating{ };

  vector3d origin{ };
  vector3d angles{ };

  std::array< float, 24 > poses{ };
  std::array< float, 24 > poses_world{ };

  void store( c_csplayer* player, matrix3x4_t* matrix, int mask );
  void setup( );
};