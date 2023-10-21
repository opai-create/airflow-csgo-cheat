#pragma once
#include "../tools/math.h"
#include "../tools/memory/memory.h"
#include "../tools/utils_macro.h"

enum fsn_stages_t
{
  frame_start = 0,
  frame_net_update_start = 1,
  frame_net_update_postdataupdate_start = 2,
  frame_net_update_postdataupdate_end = 3,
  frame_net_update_end = 4,
  frame_render_start = 5,
  frame_render_end = 6,
};

enum ClassID
{
  CAI_BaseNPC,
  CAK47,
  CBaseAnimating,
  CBaseAnimatingOverlay,
  CBaseAttributableItem,
  CBaseButton,
  CBaseCombatCharacter,
  CBaseCombatWeapon,
  CBaseCSGrenade,
  CBaseCSGrenadeProjectile,
  CBaseDoor,
  CBaseEntity,
  CBaseFlex,
  CBaseGrenade,
  CBaseParticleEntity,
  CBasePlayer,
  CBasePropDoor,
  CBaseTeamObjectiveResource,
  CBaseTempEntity,
  CBaseToggle,
  CBaseTrigger,
  CBaseViewModel,
  CBaseVPhysicsTrigger,
  CBaseWeaponWorldModel,
  CBeam,
  CBeamSpotlight,
  CBoneFollower,
  CBreakableProp,
  CBreakableSurface,
  CC4,
  CCascadeLight,
  CChicken,
  CColorCorrection,
  CColorCorrectionVolume,
  CCSGameRulesProxy,
  CCSPlayer,
  CCSPlayerResource,
  CCSRagdoll,
  CCSTeam,
  CDEagle,
  CDecoyGrenade,
  CDecoyProjectile,
  CDynamicLight,
  CDynamicProp,
  CEconEntity,
  CEconWearable,
  CEmbers,
  CEntityDissolve,
  CEntityFlame,
  CEntityFreezing,
  CEntityParticleTrail,
  CEnvAmbientLight,
  CEnvDetailController,
  CEnvDOFController,
  CEnvParticleScript,
  CEnvProjectedTexture,
  CEnvQuadraticBeam,
  CEnvScreenEffect,
  CEnvScreenOverlay,
  CEnvTonemapController,
  CEnvWind,
  CFEPlayerDecal,
  CFireCrackerBlast,
  CFireSmoke,
  CFireTrail,
  CFish,
  CFlashbang,
  CFogController,
  CFootstepControl,
  CFunc_Dust,
  CFunc_LOD,
  CFuncAreaPortalWindow,
  CFuncBrush,
  CFuncConveyor,
  CFuncLadder,
  CFuncMonitor,
  CFuncMoveLinear,
  CFuncOccluder,
  CFuncReflectiveGlass,
  CFuncRotating,
  CFuncSmokeVolume,
  CFuncTrackTrain,
  CGameRulesProxy,
  CHandleTest,
  CHEGrenade,
  CHostage,
  CHostageCarriableProp,
  CIncendiaryGrenade,
  CInferno,
  CInfoLadderDismount,
  CInfoOverlayAccessor,
  CItem_Healthshot,
  CItemDogtags,
  CKnife,
  CKnifeGG,
  CLightGlow,
  CMaterialModifyControl,
  CMolotovGrenade,
  CMolotovProjectile,
  CMovieDisplay,
  CParticleFire,
  CParticlePerformanceMonitor,
  CParticleSystem,
  CPhysBox,
  CPhysBoxMultiplayer,
  CPhysicsProp,
  CPhysicsPropMultiplayer,
  CPhysMagnet,
  CPlantedC4,
  CPlasma,
  CPlayerResource,
  CPointCamera,
  CPointCommentaryNode,
  CPointWorldText,
  CPoseController,
  CPostProcessController,
  CPrecipitation,
  CPrecipitationBlocker,
  CPredictedViewModel,
  CProp_Hallucination,
  CPropDoorRotating,
  CPropJeep,
  CPropVehicleDriveable,
  CRagdollManager,
  CRagdollProp,
  CRagdollPropAttached,
  CRopeKeyframe,
  CSCAR17,
  CSceneEntity,
  CSensorGrenade,
  CSensorGrenadeProjectile,
  CShadowControl,
  CSlideshowDisplay,
  CSmokeGrenade,
  CSmokeGrenadeProjectile,
  CSmokeStack,
  CSpatialEntity,
  CSpotlightEnd,
  CSprite,
  CSpriteOriented,
  CSpriteTrail,
  CStatueProp,
  CSteamJet,
  CSun,
  CSunlightShadowControl,
  CTeam,
  CTeamplayRoundBasedRulesProxy,
  CTEArmorRicochet,
  CTEBaseBeam,
  CTEBeamEntPoint,
  CTEBeamEnts,
  CTEBeamFollow,
  CTEBeamLaser,
  CTEBeamPoints,
  CTEBeamRing,
  CTEBeamRingPoint,
  CTEBeamSpline,
  CTEBloodSprite,
  CTEBloodStream,
  CTEBreakModel,
  CTEBSPDecal,
  CTEBubbles,
  CTEBubbleTrail,
  CTEClientProjectile,
  CTEDecal,
  CTEDust,
  CTEDynamicLight,
  CTEEffectDispatch,
  CTEEnergySplash,
  CTEExplosion,
  CTEFireBullets,
  CTEFizz,
  CTEFootprintDecal,
  CTEFoundryHelpers,
  CTEGaussExplosion,
  CTEGlowSprite,
  CTEImpact,
  CTEKillPlayerAttachments,
  CTELargeFunnel,
  CTEMetalSparks,
  CTEMuzzleFlash,
  CTEParticleSystem,
  CTEPhysicsProp,
  CTEPlantBomb,
  CTEPlayerAnimEvent,
  CTEPlayerDecal,
  CTEProjectedDecal,
  CTERadioIcon,
  CTEShatterSurface,
  CTEShowLine,
  CTesla,
  CTESmoke,
  CTESparks,
  CTESprite,
  CTESpriteSpray,
  CTest_ProxyToggle_Networkable,
  CTestTraceline,
  CTEWorldDecal,
  CTriggerPlayerMovement,
  CTriggerSoundOperator,
  CVGuiScreen,
  CVoteController,
  CWaterBullet,
  CWaterLODControl,
  CWeaponAug,
  CWeaponAWP,
  CWeaponBaseItem,
  CWeaponBizon,
  CWeaponCSBase,
  CWeaponCSBaseGun,
  CWeaponCycler,
  CWeaponElite,
  CWeaponFamas,
  CWeaponFiveSeven,
  CWeaponG3SG1,
  CWeaponGalil,
  CWeaponGalilAR,
  CWeaponGlock,
  CWeaponHKP2000,
  CWeaponM249,
  CWeaponM3,
  CWeaponM4A1,
  CWeaponMAC10,
  CWeaponMag7,
  CWeaponMP5Navy,
  CWeaponMP7,
  CWeaponMP9,
  CWeaponNegev,
  CWeaponNOVA,
  CWeaponP228,
  CWeaponP250,
  CWeaponP90,
  CWeaponSawedoff,
  CWeaponSCAR20,
  CWeaponScout,
  CWeaponSG550,
  CWeaponSG552,
  CWeaponSG556,
  CWeaponSSG08,
  CWeaponTaser,
  CWeaponTec9,
  CWeaponTMP,
  CWeaponUMP45,
  CWeaponUSP,
  CWeaponXM1014,
  CWorld,
  CWorldVguiText,
  DustTrail,
  MovieExplosion,
  ParticleSmokeGrenade,
  RocketTrail,
  SmokeTrail,
  SporeExplosion,
  SporeTrail,
};

enum send_prop_type_t
{
  _int = 0,
  _float,
  _vec,
  _vec_xy,
  _string,
  _array,
  _data_table,
  _int_64,
};

class d_variant;
class c_recv_table;
class c_recv_prop;
class c_recv_proxy_data;

using recv_var_proxy_fn = void ( * )( const c_recv_proxy_data* data, void* struct_ptr, void* out_ptr );
using array_length_recv_proxy_fn = void ( * )( void* struct_ptr, int object_id, int current_array_length );
using data_table_recv_var_proxy_fn = void ( * )( const c_recv_prop* prop, void** out_ptr, void* data_ptr, int object_id );

class d_variant
{
public:
  union
  {
    float flt_;
    long int_;
    char* string;
    void* data;
    float vector [ 3 ];
    __int64 int64;
  };

  send_prop_type_t type{ };
};

class c_recv_proxy_data
{
public:
  const c_recv_prop* recv_prop{ };
  d_variant value{ };
  int element_index{ };
  int object_id{ };
};

class c_recv_prop
{
public:
  char* prop_name{ };
  send_prop_type_t prop_type{ };
  int prop_flags{ };
  int buffer_size{ };
  int is_inside_of_array{ };
  const void* extra_data_ptr{ };
  c_recv_prop* array_prop{ };
  array_length_recv_proxy_fn array_length_proxy{ };
  recv_var_proxy_fn proxy_fn{ };
  data_table_recv_var_proxy_fn data_table_proxy_fn{ };
  c_recv_table* data_table{ };
  int offset{ };
  int element_stride{ };
  int elements_count{ };
  const char* parent_array_prop_name{ };
};

class c_recv_table
{
public:
  c_recv_prop* props{ };
  int props_count{ };
  void* decoder_ptr{ };
  char* table_name{ };
  bool is_initialized{ };
  bool is_in_main_list{ };
};

class c_client_class;
class i_networkable;

using create_client_class_fn = void* ( * )( int, int );
using create_event_fn = void* ( * )( );

class c_client_class
{
public:
  create_client_class_fn create_fn{ };
  create_event_fn create_event_fn{ };
  char* network_name{ };
  c_recv_table* recvtable_ptr{ };
  c_client_class* next_ptr{ };
  ClassID class_id{ };
};

class c_view_setup
{
public:
  int x{ }, x_old{ };
  int y{ }, y_old{ };
  int width{ };
  int width_old{ };
  int height{ };
  int height_old{ };
  bool ortho{ };
  float ortho_left{ };
  float ortho_top{ };
  float ortho_right{ };
  float ortho_bottom{ };
  bool b_custoview_matrix{ };
  matrix3x4_t custoview_matrix{ };

  padding( 0x48 );

  float fov{ };
  float fov_viewmodel{ };
  vector3d origin{ };
  vector3d angles{ };
  float near_{ };
  float far_{ };
  float near_viewmodel{ };
  float far_viewmodel{ };
  float aspect_ratio{ };
  float near_blur_depth{ };
  float near_focus_depth{ };
  float far_focus_depth{ };
  float far_blur_depth{ };
  float near_blur_radius{ };
  float far_blur_radius{ };
  int do_f_quality{ };
  int motion_blur_mode{ };
  float shutter_time{ };
  vector3d shutter_open_position{ };
  vector3d shutter_open_angles{ };
  vector3d shutter_close_position{ };
  vector3d shutter_close_angles{ };
  float off_center_top{ };
  float off_center_bottom{ };
  float off_center_left{ };
  float off_center_right{ };
  int edge_blur{ };
};

class c_base_client_dll
{
public:
  bool chat_opened( )
  {
    using fn = bool( __thiscall* )( void* );
    return g_memory->getvfunc< fn >( this, 91 )( this );
  }

  bool write_user_cmd_delta_to_buffer( int slot, void* buf, int from, int to, bool is_new_cmd )
  {
    using fn = bool( __thiscall* )( c_base_client_dll*, int, void*, int, int, bool );
    return g_memory->getvfunc< fn >( this, 23 )( this, slot, buf, from, to, is_new_cmd );
  }

  c_client_class* get_client_classes( )
  {
    using fn = c_client_class*( __thiscall* )( c_base_client_dll* );
    return g_memory->getvfunc< fn >( this, 8 )( this );
  }
};

class CCSGO_HudChat
{
public:
  padding( 13 );
  bool chat_opened;
};