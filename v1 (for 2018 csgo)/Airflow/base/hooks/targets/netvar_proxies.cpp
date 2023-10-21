#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../functions/config_vars.h"

#include "../../../functions/skins/skins.h"

#include "../../../functions/features.h"

#include <string>

namespace tr::netvar_proxies
{
  void __cdecl viewmodel_sequence( c_recv_proxy_data* data, void* entity, void* out )
  {
    if( g_ctx.local && g_ctx.local->is_alive( ) && interfaces::client_state->delta_tick != -1 && g_cfg.skins.skin_weapon [ weapon_cfg_knife ].enable )
    {
      auto base_entity = ( c_baseentity* )entity;

      auto owner = ( c_baseentity* )interfaces::entity_list->get_entity_handle( base_entity->viewmodel_owner( ) );
      if( !owner || owner->index( ) != g_ctx.local->index( ) )
        return original_sequence( data, entity, out );

      auto view_model_weapon = base_entity->get_view_model_weapon( );
      if( !view_model_weapon )
        return original_sequence( data, entity, out );

      auto client_class = view_model_weapon->get_client_class( );
      if( !client_class || client_class->class_id != CKnife )
        return original_sequence( data, entity, out );

      data->value.int_ = skin_changer::correct_sequence( view_model_weapon->item_definition_index( ), data->value.int_ );
      original_sequence( data, entity, out );
    }
    else
      original_sequence( data, entity, out );
  }
}