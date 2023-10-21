#pragma once
#include "../interfaces/client.h"
#include "math.h"
#include "utils_macro.h"

namespace offsets
{
  inline uintptr_t m_fFlags{ };
  inline uintptr_t m_iHealth{ };
  inline uintptr_t m_lifeState{ };
  inline uintptr_t m_vecVelocity{ };
  inline uintptr_t m_bIsScoped{ };
  inline uintptr_t m_bHasHeavyArmor{ };
  inline uintptr_t m_hActiveWeapon{ };
  inline uintptr_t m_hWeaponWorldModel{ };
  inline uintptr_t m_Item{ };
  inline uintptr_t m_nTickBase{ };
  inline uintptr_t m_vphysicsCollisionState{ };
  inline uintptr_t m_nFinalPredictedTick{ };
  inline uintptr_t m_aimPunchAngle{ };
  inline uintptr_t m_aimPunchAngleVel{ };
  inline uintptr_t m_flFallVelocity{ };
  inline uintptr_t m_vecBaseVelocity{ };
  inline uintptr_t m_viewPunchAngle{ };
  inline uintptr_t m_iObserverMode{ };
  inline uintptr_t m_iMoveState{ };
  inline uintptr_t m_bClientSideAnimation{ };
  inline uintptr_t m_bIsWalking{ };
  inline uintptr_t m_flPoseParameter{ };
  inline uintptr_t m_vecOrigin{ };
  inline uintptr_t m_angEyeAngles{ };
  inline uintptr_t m_iTeamNum{ };
  inline uintptr_t m_bGunGameImmunity{ };
  inline uintptr_t m_flSimulationTime{ };
  inline uintptr_t m_flVelocityModifier{ };
  inline uintptr_t m_fLastShotTime{ };
  inline uintptr_t m_flFlashtime{ };
  inline uintptr_t m_flFlashMaxAlpha{ };
  inline uintptr_t m_iItemDefinitionIndex{ };
  inline uintptr_t m_flNextPrimaryAttack{ };
  inline uintptr_t m_flNextSecondaryAttack{ };
  inline uintptr_t m_flPostponeFireReadyTime{ };
  inline uintptr_t m_hViewModel{ };
  inline uintptr_t m_nViewModelIndex{ };
  inline uintptr_t m_nSequence{ };
  inline uintptr_t m_nAnimationParity{ };
  inline uintptr_t m_hWeapon{ };
  inline uintptr_t m_CachedBoneData{ };
  inline uintptr_t m_vecViewOffset{ };
  inline uintptr_t deadflag{ };
  inline uintptr_t m_CollisionGroup{ };
  inline uintptr_t m_bHasHelmet{ };
  inline uintptr_t m_ArmorValue{ };
  inline uintptr_t m_iAccount{ };
  inline uintptr_t m_iClip1{ };
  inline uintptr_t m_bSpotted{ };
  inline uintptr_t m_nForceBone{ };
  inline uintptr_t m_nHitboxSet{ };
  inline uintptr_t m_vecMins{ };
  inline uintptr_t m_vecMaxs{ };
  inline uintptr_t m_hOwnerEntity{ };
  inline uintptr_t m_fireCount{ };
  inline uintptr_t m_bFireIsBurning{ };
  inline uintptr_t m_fireXDelta{ };
  inline uintptr_t m_fireYDelta{ };
  inline uintptr_t m_fireZDelta{ };
  inline uintptr_t m_nBombSite{ };
  inline uintptr_t m_bDidSmokeEffect{ };
  inline uintptr_t m_nSmokeEffectTickBegin{ };
  inline uintptr_t m_hBombDefuser{ };
  inline uintptr_t m_flDefuseCountDown{ };
  inline uintptr_t m_flC4Blow{ };
  inline uintptr_t m_bBombTicking{ };
  inline uintptr_t m_bBombDefused{ };
  inline uintptr_t m_bHasDefuser{ };
  inline uintptr_t m_bIsDefusing{ };
  inline uintptr_t m_flTimerLength{ };
  inline uintptr_t m_hMoveParent{ };
  inline uintptr_t m_hGroundEntity{ };
  inline uintptr_t m_iFOV{ };
  inline uintptr_t m_flFOVRate{ };
  inline uintptr_t m_iFOVStart{ };
  inline uintptr_t m_iDefaultFOV{ };
  inline uintptr_t m_zoomLevel{ };
  inline uintptr_t m_skyBoxArea{ };
  inline uintptr_t m_fogStart{ };
  inline uintptr_t m_fogEnd{ };
  inline uintptr_t m_fogColorPrimary{ };
  inline uintptr_t m_fAccuracyPenalty{ };
  inline uintptr_t m_flRecoilIndex{ };
  inline uintptr_t m_flDuckAmount{ };
  inline uintptr_t m_flDuckSpeed{ };
  inline uintptr_t m_flMaxSpeed{ };
  inline uintptr_t m_iPing{ };
  inline uintptr_t m_iPlayerC4{ };
  inline uintptr_t m_flNextAttack{ };
  inline uintptr_t m_iBurstShotsRemaining{ };
  inline uintptr_t m_fNextBurstShot{ };
  inline uintptr_t m_bPinPulled{ };
  inline uintptr_t m_fThrowTime{ };
  inline uintptr_t m_flThrowStrength{ };
  inline uintptr_t m_hThrower{ };
  inline uintptr_t m_nExplodeEffectTickBegin{ };
  inline uintptr_t m_MoveType{ };
  inline uintptr_t m_bUseCustomAutoExposureMin{ };
  inline uintptr_t m_bUseCustomAutoExposureMax{ };
  inline uintptr_t m_flCustomAutoExposureMin{ };
  inline uintptr_t m_flCustomAutoExposureMax{ };
  inline uintptr_t m_bUseCustomBloomScale{ };
  inline uintptr_t m_flCustomBloomScale{ };
  inline uintptr_t m_angAbsRotation{ };
  inline uintptr_t m_flCycle{ };
  inline uintptr_t m_flAnimTime{ };
  inline uintptr_t m_iAddonBits{ };
  inline uintptr_t m_flThirdpersonRecoil{ };
  inline uintptr_t m_flLowerBodyYawTarget{ };
  inline uintptr_t m_hEffectEntity{ };
  inline uintptr_t m_bIsAutoaimTarget{ };
  inline uintptr_t m_hLightingOrigin{ };
  inline uintptr_t m_hConstraintEntity{ };
  inline uintptr_t m_hObserverTarget{ };
  inline uintptr_t m_bWaitForNoAttack{ };
  inline uintptr_t m_iPlayerState{ };
  inline uintptr_t m_iTimeOfDay{ };
  inline uintptr_t m_hViewEntity{ };
  inline uintptr_t m_nFallbackStatTrak{ };
  inline uintptr_t m_nFallbackPaintKit{ };
  inline uintptr_t m_nFallbackSeed{ };
  inline uintptr_t m_flFallbackWear{ };
  inline uintptr_t m_iAccountID{ };
  inline uintptr_t m_iItemIDHigh{ };
  inline uintptr_t m_szCustomName{ };
  inline uintptr_t m_OriginalOwnerXuidLow{ };
  inline uintptr_t m_OriginalOwnerXuidHigh{ };
  inline uintptr_t m_iEntityQuality{ };
  inline uintptr_t m_hMyWearables{ };
  inline uintptr_t m_nModelIndex{ };
  inline uintptr_t m_vecForce{ };
  inline uintptr_t m_vecRagdollVelocity{ };
  inline uintptr_t m_hRagdoll{ };
  inline uintptr_t m_bIsBroken{ };
  inline uintptr_t m_hPlayer{ };
  inline uintptr_t m_hOwner{ };
  inline uintptr_t m_hMyWeapons{ };
  inline uintptr_t m_bStrafing{ };
}

class c_netvar_manager
{
private:
  std::vector< c_recv_table* > recv_tables{ };

  int get_prop( c_recv_table* table, uint32_t prop_name, c_recv_prop** prop = 0 );
  c_recv_table* get_table( uint32_t table_name );

public:
  void init( );
  uint32_t get_offset( uint32_t table_hash, uint32_t prop_hash );
  bool hook_prop( uint32_t table_hash, uint32_t prop_name, recv_var_proxy_fn fn, recv_var_proxy_fn& orig, bool store = false );
};

declare_feature_ptr( netvar_manager );

namespace netvars
{
  extern void init( );
}