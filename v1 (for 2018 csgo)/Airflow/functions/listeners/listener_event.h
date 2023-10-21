#pragma once
#include "../../base/interfaces/entity_list.h"
#include "../../base/sdk/entity.h"

#include "../../base/interfaces/event_manager.h"

#include <vector>

class c_event_listener: public c_game_event_listener2
{
private:
  std::vector< std::string > event_list = {
    xor_str( "player_hurt" ),
    xor_str( "item_purchase" ),
    xor_str( "bullet_impact" ),
    xor_str( "bomb_planted" ),
    xor_str( "bomb_begindefuse" ),
    xor_str( "weapon_fire" ),
    xor_str( "round_start" ),
    xor_str( "round_prestart" ),
    xor_str( "round_end" ),
  };

public:
  virtual void fire_game_event( c_game_event* event ) override;
  virtual int get_event_debug_id( ) override
  {
    return 42;
  }

  void init_events( );
  void remove_events( );
};