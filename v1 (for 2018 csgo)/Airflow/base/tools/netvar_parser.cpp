#include "netvar_parser.h"

#include "memory/memory.h"
#include "../global_context.h"

create_feature_ptr( netvar_manager );

void c_netvar_manager::init( )
{
  recv_tables.clear( );

  auto client_class = interfaces::client->get_client_classes( );
  if( !client_class )
    return;

  while( client_class != nullptr )
  {
    recv_tables.push_back( client_class->recvtable_ptr );
    client_class = client_class->next_ptr;
  }
}

uint32_t c_netvar_manager::get_offset( uint32_t table_hash, uint32_t prop_hash )
{
  return get_prop( get_table( table_hash ), prop_hash );
}

int c_netvar_manager::get_prop( c_recv_table* table, uint32_t prop_hash, c_recv_prop** prop )
{
  if( !table )
    return 0;

  int offset = 0;
  for( int i = 0; i < table->props_count; ++i )
  {
    auto recv_prop = &table->props [ i ];
    auto child = recv_prop->data_table;

    if( child != nullptr && child->props_count > 0 )
    {
      int extra_offset = get_prop( child, prop_hash, prop );
      if( extra_offset )
        offset += recv_prop->offset + extra_offset;
    }

    if( _fnva1( recv_prop->prop_name ) != prop_hash )
      continue;

    if( prop )
      *prop = recv_prop;

    return recv_prop->offset + offset;
  }

  return offset;
}

c_recv_table* c_netvar_manager::get_table( uint32_t table_hash )
{
  if( recv_tables.empty( ) )
    return nullptr;

  for( auto table : recv_tables )
  {
    if( !table )
      continue;

    if( _fnva1( table->table_name ) == table_hash )
      return table;
  }

  return nullptr;
}

bool c_netvar_manager::hook_prop( uint32_t table_hash, uint32_t prop_name, recv_var_proxy_fn fn, recv_var_proxy_fn& orig, bool store )
{
  auto table = get_table( table_hash );
  if( !table )
    return false;

  c_recv_prop* prop = nullptr;
  get_prop( table, prop_name, &prop );
  if( !prop )
    return false;

  auto old_prop_proxy_fn = prop->proxy_fn;
  if( store )
    orig = old_prop_proxy_fn;

  prop->proxy_fn = fn;
  return true;
}

namespace netvars
{
  __forceinline void init( )
  {
    g_netvar_manager->init( );

    offsets::m_fFlags = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_fFlags" ) );
    offsets::m_iHealth = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iHealth" ) );
    offsets::m_bIsScoped = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bIsScoped" ) );
    offsets::m_angEyeAngles = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_angEyeAngles[0]" ) );
    offsets::m_bGunGameImmunity = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bGunGameImmunity" ) );
    offsets::m_flSimulationTime = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_flSimulationTime" ) );
    offsets::m_flVelocityModifier = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_flVelocityModifier" ) );
    offsets::m_bHasHelmet = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bHasHelmet" ) );
    offsets::m_ArmorValue = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_ArmorValue" ) );
    offsets::m_iAccount = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iAccount" ) );
    offsets::m_bHasDefuser = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bHasDefuser" ) );
    offsets::m_bIsDefusing = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bIsDefusing" ) );
    offsets::m_iFOV = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iFOV" ) );
    offsets::m_iFOVStart = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iFOVStart" ) );
    offsets::m_iDefaultFOV = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iDefaultFOV" ) );
    offsets::m_flFlashtime = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_flFlashDuration" ) );
    offsets::m_flFlashMaxAlpha = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_flFlashMaxAlpha" ) );
    offsets::m_viewPunchAngle = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_viewPunchAngle" ) );
    offsets::m_iObserverMode = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iObserverMode" ) );
    offsets::m_flNextAttack = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_flNextAttack" ) );
    offsets::m_iAddonBits = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iAddonBits" ) );
    offsets::m_flThirdpersonRecoil = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_flThirdpersonRecoil" ) );
    offsets::m_flLowerBodyYawTarget = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_flLowerBodyYawTarget" ) );
    offsets::m_bIsWalking = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bIsWalking" ) );
    offsets::m_bHasHeavyArmor = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bHasHeavyArmor" ) );
    offsets::m_hObserverTarget = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_hObserverTarget" ) );
    offsets::m_bWaitForNoAttack = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bWaitForNoAttack" ) );
    offsets::m_iPlayerState = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iPlayerState" ) );
    offsets::m_aimPunchAngle = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_aimPunchAngle" ) );
    offsets::m_aimPunchAngleVel = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_aimPunchAngleVel" ) );
    offsets::m_iMoveState = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_iMoveState" ) );
    offsets::m_hRagdoll = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_hRagdoll" ) );
    offsets::m_vphysicsCollisionState = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_vphysicsCollisionState" ) );
    offsets::m_hMyWeapons = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_hMyWeapons" ) );
    offsets::m_bStrafing = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayer" ), __fnva1( "m_bStrafing" ) );

    offsets::m_vecVelocity = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_vecVelocity[0]" ) );
    offsets::m_hActiveWeapon = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_hActiveWeapon" ) );
    offsets::m_nTickBase = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_nTickBase" ) );
    offsets::m_nFinalPredictedTick = offsets::m_nTickBase + 0x4;
    offsets::m_hViewModel = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_hViewModel[0]" ) );
    offsets::m_vecViewOffset = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_vecViewOffset[0]" ) );
    offsets::m_skyBoxArea = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_skybox3d.area" ) );
    offsets::m_hGroundEntity = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_hGroundEntity" ) );
    offsets::m_flDuckAmount = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_flDuckAmount" ) );
    offsets::m_flDuckSpeed = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_flDuckSpeed" ) );
    offsets::m_flFOVRate = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_flFOVRate" ) );
    offsets::m_hEffectEntity = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_hEffectEntity" ) );
    offsets::m_bIsAutoaimTarget = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_bIsAutoaimTarget" ) );
    offsets::deadflag = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "deadflag" ) );
    offsets::m_flFallVelocity = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_flFallVelocity" ) );
    offsets::m_vecBaseVelocity = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_vecBaseVelocity" ) );
    offsets::m_flMaxSpeed = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_flMaxSpeed" ) );
    offsets::m_hConstraintEntity = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_hConstraintEntity" ) );
    offsets::m_lifeState = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_lifeState" ) );
    offsets::m_hViewEntity = g_netvar_manager->get_offset( __fnva1( "DT_BasePlayer" ), __fnva1( "m_hViewEntity" ) );

    offsets::m_bClientSideAnimation = g_netvar_manager->get_offset( __fnva1( "DT_BaseAnimating" ), __fnva1( "m_bClientSideAnimation" ) );
    offsets::m_nSequence = g_netvar_manager->get_offset( __fnva1( "DT_BaseAnimating" ), __fnva1( "m_nSequence" ) );
    offsets::m_nForceBone = g_netvar_manager->get_offset( __fnva1( "DT_BaseAnimating" ), __fnva1( "m_nForceBone" ) );
    offsets::m_nHitboxSet = g_netvar_manager->get_offset( __fnva1( "DT_BaseAnimating" ), __fnva1( "m_nHitboxSet" ) );
    offsets::m_flPoseParameter = g_netvar_manager->get_offset( __fnva1( "DT_BaseAnimating" ), __fnva1( "m_flPoseParameter" ) );
    offsets::m_hLightingOrigin = g_netvar_manager->get_offset( __fnva1( "DT_BaseAnimating" ), __fnva1( "m_hLightingOrigin" ) );

    offsets::m_flCycle = g_netvar_manager->get_offset( __fnva1( "DT_BaseAnimating" ), __fnva1( "m_flCycle" ) );

    offsets::m_vecOrigin = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_vecOrigin" ) );
    offsets::m_iTeamNum = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_iTeamNum" ) );
    offsets::m_CollisionGroup = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_CollisionGroup" ) );
    offsets::m_bSpotted = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_bSpotted" ) );
    offsets::m_vecMins = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_vecMins" ) );
    offsets::m_vecMaxs = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_vecMaxs" ) );
    offsets::m_hOwnerEntity = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_hOwnerEntity" ) );
    offsets::m_hMoveParent = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "moveparent" ) );
    offsets::m_flAnimTime = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_flAnimTime" ) );
    offsets::m_nModelIndex = g_netvar_manager->get_offset( __fnva1( "DT_BaseEntity" ), __fnva1( "m_nModelIndex" ) );

    offsets::m_fLastShotTime = g_netvar_manager->get_offset( __fnva1( "DT_WeaponCSBase" ), __fnva1( "m_fLastShotTime" ) );
    offsets::m_fAccuracyPenalty = g_netvar_manager->get_offset( __fnva1( "DT_WeaponCSBase" ), __fnva1( "m_fAccuracyPenalty" ) );
    offsets::m_flRecoilIndex = g_netvar_manager->get_offset( __fnva1( "DT_WeaponCSBase" ), __fnva1( "m_flRecoilIndex" ) );
    offsets::m_flPostponeFireReadyTime = g_netvar_manager->get_offset( __fnva1( "DT_WeaponCSBase" ), __fnva1( "m_flPostponeFireReadyTime" ) );

    offsets::m_zoomLevel = g_netvar_manager->get_offset( __fnva1( "DT_WeaponCSBaseGun" ), __fnva1( "m_zoomLevel" ) );
    offsets::m_iBurstShotsRemaining = g_netvar_manager->get_offset( __fnva1( "DT_WeaponCSBaseGun" ), __fnva1( "m_iBurstShotsRemaining" ) );

    offsets::m_fNextBurstShot = g_netvar_manager->get_offset( __fnva1( "CWeaponCSBaseGun" ), __fnva1( "m_fNextBurstShot" ) );

    offsets::m_bPinPulled = g_netvar_manager->get_offset( __fnva1( "DT_BaseCSGrenade" ), __fnva1( "m_bPinPulled" ) );
    offsets::m_fThrowTime = g_netvar_manager->get_offset( __fnva1( "DT_BaseCSGrenade" ), __fnva1( "m_fThrowTime" ) );
    offsets::m_flThrowStrength = g_netvar_manager->get_offset( __fnva1( "DT_BaseCSGrenade" ), __fnva1( "m_flThrowStrength" ) );
    offsets::m_hThrower = g_netvar_manager->get_offset( __fnva1( "DT_BaseCSGrenade" ), __fnva1( "m_hThrower" ) );

    offsets::m_iItemDefinitionIndex = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_iItemDefinitionIndex" ) );
    offsets::m_nFallbackStatTrak = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_nFallbackStatTrak" ) );
    offsets::m_nFallbackPaintKit = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_nFallbackPaintKit" ) );
    offsets::m_nFallbackSeed = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_nFallbackSeed" ) );
    offsets::m_flFallbackWear = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_flFallbackWear" ) );
    offsets::m_iAccountID = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_iAccountID" ) );
    offsets::m_iItemIDHigh = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_iItemIDHigh" ) );
    offsets::m_szCustomName = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_szCustomName" ) );
    offsets::m_OriginalOwnerXuidLow = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_OriginalOwnerXuidLow" ) );
    offsets::m_OriginalOwnerXuidHigh = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_OriginalOwnerXuidHigh" ) );
    offsets::m_iEntityQuality = g_netvar_manager->get_offset( __fnva1( "DT_BaseAttributableItem" ), __fnva1( "m_iEntityQuality" ) );
    offsets::m_hMyWearables = g_netvar_manager->get_offset( __fnva1( "DT_BaseCombatCharacter" ), __fnva1( "m_hMyWearables" ) );

    offsets::m_nViewModelIndex = g_netvar_manager->get_offset( __fnva1( "DT_BaseViewModel" ), __fnva1( "m_nViewModelIndex" ) );
    offsets::m_nAnimationParity = g_netvar_manager->get_offset( __fnva1( "DT_BaseViewModel" ), __fnva1( "m_nAnimationParity" ) );
    offsets::m_hWeapon = g_netvar_manager->get_offset( __fnva1( "DT_BaseViewModel" ), __fnva1( "m_hWeapon" ) );
    offsets::m_hOwner = g_netvar_manager->get_offset( __fnva1( "DT_BaseViewModel" ), __fnva1( "m_hOwner" ) );

    offsets::m_iClip1 = g_netvar_manager->get_offset( __fnva1( "DT_BaseCombatWeapon" ), __fnva1( "m_iClip1" ) );
    offsets::m_flNextPrimaryAttack = g_netvar_manager->get_offset( __fnva1( "DT_BaseCombatWeapon" ), __fnva1( "m_flNextPrimaryAttack" ) );
    offsets::m_flNextSecondaryAttack = g_netvar_manager->get_offset( __fnva1( "DT_BaseCombatWeapon" ), __fnva1( "m_flNextSecondaryAttack" ) );
    offsets::m_hWeaponWorldModel = g_netvar_manager->get_offset( __fnva1( "DT_BaseCombatWeapon" ), __fnva1( "m_hWeaponWorldModel" ) );
    offsets::m_Item = g_netvar_manager->get_offset( __fnva1( "DT_BaseCombatWeapon" ), __fnva1( "m_Item" ) );

    offsets::m_fireCount = g_netvar_manager->get_offset( __fnva1( "DT_Inferno" ), __fnva1( "m_fireCount" ) );
    offsets::m_bFireIsBurning = g_netvar_manager->get_offset( __fnva1( "DT_Inferno" ), __fnva1( "m_bFireIsBurning" ) );
    offsets::m_fireXDelta = g_netvar_manager->get_offset( __fnva1( "DT_Inferno" ), __fnva1( "m_fireXDelta" ) );
    offsets::m_fireYDelta = g_netvar_manager->get_offset( __fnva1( "DT_Inferno" ), __fnva1( "m_fireYDelta" ) );
    offsets::m_fireZDelta = g_netvar_manager->get_offset( __fnva1( "DT_Inferno" ), __fnva1( "m_fireZDelta" ) );

    offsets::m_bDidSmokeEffect = g_netvar_manager->get_offset( __fnva1( "DT_SmokeGrenadeProjectile" ), __fnva1( "m_bDidSmokeEffect" ) );
    offsets::m_nSmokeEffectTickBegin = g_netvar_manager->get_offset( __fnva1( "DT_SmokeGrenadeProjectile" ), __fnva1( "m_nSmokeEffectTickBegin" ) );

    offsets::m_nExplodeEffectTickBegin = g_netvar_manager->get_offset( __fnva1( "DT_BaseCSGrenadeProjectile" ), __fnva1( "m_nExplodeEffectTickBegin" ) );

    offsets::m_hBombDefuser = g_netvar_manager->get_offset( __fnva1( "DT_PlantedC4" ), __fnva1( "m_hBombDefuser" ) );
    offsets::m_flDefuseCountDown = g_netvar_manager->get_offset( __fnva1( "DT_PlantedC4" ), __fnva1( "m_flDefuseCountDown" ) );
    offsets::m_flC4Blow = g_netvar_manager->get_offset( __fnva1( "DT_PlantedC4" ), __fnva1( "m_flC4Blow" ) );
    offsets::m_bBombTicking = g_netvar_manager->get_offset( __fnva1( "DT_PlantedC4" ), __fnva1( "m_bBombTicking" ) );
    offsets::m_bBombDefused = g_netvar_manager->get_offset( __fnva1( "DT_PlantedC4" ), __fnva1( "m_bBombDefused" ) );
    offsets::m_flTimerLength = g_netvar_manager->get_offset( __fnva1( "DT_PlantedC4" ), __fnva1( "m_flTimerLength" ) );
    offsets::m_nBombSite = g_netvar_manager->get_offset( __fnva1( "DT_PlantedC4" ), __fnva1( "m_nBombSite" ) );

    offsets::m_fogStart = g_netvar_manager->get_offset( __fnva1( "DT_FogController" ), __fnva1( "m_fog.start" ) );
    offsets::m_fogEnd = g_netvar_manager->get_offset( __fnva1( "DT_FogController" ), __fnva1( "m_fog.end" ) );
    offsets::m_fogColorPrimary = g_netvar_manager->get_offset( __fnva1( "DT_FogController" ), __fnva1( "m_fog.colorPrimary" ) );

    offsets::m_iPing = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayerResource" ), __fnva1( "m_iPing" ) );
    offsets::m_iPlayerC4 = g_netvar_manager->get_offset( __fnva1( "DT_CSPlayerResource" ), __fnva1( "m_iPlayerC4" ) );

    offsets::m_MoveType = g_netvar_manager->get_offset( __fnva1( "C_BaseEntity" ), __fnva1( "m_MoveType" ) );
    offsets::m_angAbsRotation = g_netvar_manager->get_offset( __fnva1( "C_BaseEntity" ), __fnva1( "m_angAbsRotation" ) );
    offsets::m_iTimeOfDay = g_netvar_manager->get_offset( __fnva1( "DT_World" ), __fnva1( "m_iTimeOfDay" ) );

    offsets::m_bUseCustomAutoExposureMin = g_netvar_manager->get_offset( __fnva1( "DT_EnvTonemapController" ), __fnva1( "m_bUseCustomAutoExposureMin" ) );
    offsets::m_bUseCustomAutoExposureMax = g_netvar_manager->get_offset( __fnva1( "DT_EnvTonemapController" ), __fnva1( "m_bUseCustomAutoExposureMax" ) );
    offsets::m_flCustomAutoExposureMin = g_netvar_manager->get_offset( __fnva1( "DT_EnvTonemapController" ), __fnva1( "m_flCustomAutoExposureMin" ) );
    offsets::m_flCustomAutoExposureMax = g_netvar_manager->get_offset( __fnva1( "DT_EnvTonemapController" ), __fnva1( "m_flCustomAutoExposureMax" ) );
    offsets::m_bUseCustomBloomScale = g_netvar_manager->get_offset( __fnva1( "DT_EnvTonemapController" ), __fnva1( "m_bUseCustomBloomScale" ) );
    offsets::m_flCustomBloomScale = g_netvar_manager->get_offset( __fnva1( "DT_EnvTonemapController" ), __fnva1( "m_flCustomBloomScale" ) );

    offsets::m_vecForce = g_netvar_manager->get_offset( __fnva1( "DT_CSRagdoll" ), __fnva1( "m_vecForce" ) );
    offsets::m_vecRagdollVelocity = g_netvar_manager->get_offset( __fnva1( "DT_CSRagdoll" ), __fnva1( "m_vecRagdollVelocity" ) );

    offsets::m_bIsBroken = g_netvar_manager->get_offset( __fnva1( "DT_BreakableSurface" ), __fnva1( "m_bIsBroken" ) );

    offsets::m_hPlayer = g_netvar_manager->get_offset( __fnva1( "DT_CSRagdoll" ), __fnva1( "m_hPlayer" ) );
  }
}