#pragma once
#include "extra/movement.h"
#include "extra/utils.h"
#include "extra/world_modulation.h"

#include "listeners/listener_entity.h"
#include "listeners/listener_event.h"

#include "anti hit/fake_lag.h"
#include "anti hit/exploits.h"
#include "anti hit/anti_aim.h"

#include "visuals/visuals.h"
#include "visuals/local_player_visuals.h"
#include "visuals/grenade_warning.h"
#include "visuals/glow.h"
#include "visuals/esp_store.h"
#include "visuals/esp_player.h"
#include "visuals/esp_weapon.h"
#include "visuals/chams.h"
#include "visuals/event/event_visuals.h"
#include "visuals/event/event_logger.h"

#include "ragebot/animfix.h"
#include "ragebot/local_animfix.h"
#include "ragebot/rage_tools.h"
#include "ragebot/engine_prediction.h"
#include "ragebot/autowall.h"
#include "ragebot/ragebot.h"

#include "menu/menu.h"

declare_feature_ptr( world_modulation );
declare_feature_ptr( utils );
declare_feature_ptr( movement );

declare_feature_ptr( event_listener );
declare_feature_ptr( listener_entity );

declare_feature_ptr( fake_lag );
declare_feature_ptr( tickbase );
declare_feature_ptr( exploits );
declare_feature_ptr( anti_aim );
declare_feature_ptr( ping_spike );

declare_feature_ptr( visuals_wrapper );
declare_feature_ptr( local_visuals );
declare_feature_ptr( grenade_warning );
declare_feature_ptr( glow_esp );
declare_feature_ptr( esp_store );
declare_feature_ptr( player_esp );
declare_feature_ptr( weapon_esp );
declare_feature_ptr( chams );
declare_feature_ptr( event_visuals );
declare_feature_ptr( event_logger );

declare_feature_ptr( animation_fix );
declare_feature_ptr( local_animation_fix );
declare_feature_ptr( engine_prediction );
declare_feature_ptr( auto_wall );
declare_feature_ptr( rage_bot );

declare_feature_ptr( menu );