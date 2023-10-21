#pragma once
#include "../../base/hooks/hooks.h"

#include "../../base/interfaces/entity_list.h"
#include "../../base/sdk/entity.h"

#include <vector>

enum listener_ent_t
{
  ent_invalid = -1,
  ent_player,
  ent_weapon,
  ent_fog,
  ent_tonemap,
  ent_light,
  ent_ragdoll,
  ent_c4,
  ent_max,
};

bool is_bomb( int class_id );

struct listened_entity_t
{
  c_baseentity* m_entity{ };
  int m_idx{ };
  int m_class_id{ };

  listened_entity_t( ) = default;

  listened_entity_t( c_baseentity* ent )
  {
    m_entity = ent;
    m_idx = ent->index( );
    m_class_id = ent->get_client_class( )->class_id;
  }
};

class c_listener_entity: public c_entity_listener
{
private:
  std::array< std::vector< listened_entity_t >, ent_max > ent_lists{ };

public:
  const std::vector< listened_entity_t >& get_entity( int type );

  void on_entity_created( c_baseentity* entity ) override;
  void on_entity_deleted( c_baseentity* entity ) override;

  void init_entities( );
  void remove_entities( );
};