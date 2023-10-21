#include "includes.h"

#include "base/global_context.h"
#include "base/tools/memory/displacement.h"
#include "base/tools/memory/memory.h"
#include "base/tools/netvar_parser.h"
#include "base/tools/key_states.h"
#include "base/other/game_functions.h"
#include "base/hooks/hooks.h"
#include "base/tools/render.h"
#include "base/tools/threads.h"

#include "functions/features.h"

#include "functions/config_system.h"

#include "functions/ragebot/ragebot.h"
#include <ShlObj.h>

#include "functions/skins/skins.h"

#include "additionals/tinyformat.h"
#include <fstream>
#include <format>

// #define DEBUG_LOG

std::ofstream file_stream;

FILE* stream = NULL;
HMODULE cheat_module = NULL;

#ifdef _DEBUG
void CreateConsole( )
{
  AllocConsole( );
  freopen_s( &stream, ( "CONIN$" ), ( "r" ), stdin );
  freopen_s( &stream, ( "CONOUT$" ), ( "w" ), stdout );
  freopen_s( &stream, ( "CONOUT$" ), ( "w" ), stderr );
}

void DestroyConsole( )
{
  HWND console = GetConsoleWindow( );
  FreeConsole( );
  PostMessage( console, WM_CLOSE, 0, 0 );
  fclose( stream );
}
#else
#endif

void wait_modules( )
{
  while( !( g_ctx.window = FindWindowA( xor_c( "Valve001" ), NULL ) ) )
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

  while( !GetModuleHandleA( xor_c( "serverbrowser.dll" ) ) )
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
}

void update_ptrs( )
{
  update_feature_ptr( world_modulation );
  update_feature_ptr( utils );
  update_feature_ptr( movement );

  update_feature_ptr( event_listener );
  update_feature_ptr( listener_entity );

  update_feature_ptr( fake_lag );
  update_feature_ptr( tickbase );
  update_feature_ptr( exploits );
  update_feature_ptr( anti_aim );
  update_feature_ptr( ping_spike );

  update_feature_ptr( visuals_wrapper );
  update_feature_ptr( local_visuals );
  update_feature_ptr( grenade_warning );
  update_feature_ptr( glow_esp );
  update_feature_ptr( esp_store );
  update_feature_ptr( player_esp );
  update_feature_ptr( weapon_esp );
  update_feature_ptr( chams );
  update_feature_ptr( event_visuals );
  update_feature_ptr( event_logger );

  update_feature_ptr( animation_fix );
  update_feature_ptr( local_animation_fix );
  update_feature_ptr( engine_prediction );
  update_feature_ptr( auto_wall );
  update_feature_ptr( rage_bot );

  update_feature_ptr( menu );

  update_feature_ptr( netvar_manager );
  update_feature_ptr( key_states );
  update_feature_ptr( memory );
  update_feature_ptr( render );
}

int threads_count = -1;

void cheat_load( )
{
  file_stream << xor_c( "DS" ) << std::endl;

#ifdef DEBUG_LOG
  file_stream << xor_c( "UP" ) << std::endl;
#endif

  update_ptrs( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "CF" ) << std::endl;
#endif

  config::create_config_folder( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IM" ) << std::endl;
#endif

  modules::init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IP" ) << std::endl;
#endif

  patterns::init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "II" ) << std::endl;
#endif

  interfaces::init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IN" ) << std::endl;
#endif

  netvars::init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IF" ) << std::endl;
#endif

  func_ptrs::init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IX" ) << std::endl;
#endif

  xor_strs::init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IC" ) << std::endl;
#endif

  cvars::init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IM" ) << std::endl;
#endif

  g_chams->init_materials( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "ISK" ) << std::endl;
#endif

  g_ctx.sky_name = cvars::sv_skyname->string;

#ifdef DEBUG_LOG
  file_stream << xor_c( "IEN" ) << std::endl;
#endif

  g_engine_prediction->init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "ISC" ) << std::endl;
#endif

  g_render->update_screen_size( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IEN" ) << std::endl;
#endif

  g_listener_entity->init_entities( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IEV" ) << std::endl;
#endif

  g_event_listener->init_events( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "ISKI" ) << std::endl;
#endif

  skin_changer::init_parser( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "IT" ) << std::endl;
#endif

  auto ptr = patterns::thread_id_allocated.get< bool* >( );

  if( ptr )
  {
    int threads_allocated = 1;
    for( int i = 1; i < 128; ++i )
    {
      if( ptr [ i ] )
        threads_allocated++;
      else
        break;
    }

    threads_count = std::clamp( 32 - threads_allocated - 1, 3, 32 );
  }
  else
    threads_count = 3;

#ifndef _DEBUG
  g_thread_pool->create_threads( threads_count );
#endif

#ifdef DEBUG_LOG
  file_stream << xor_c( "IHK" ) << std::endl;
#endif

  hooks::init( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "C" ) << std::endl;
#endif

  g_ctx.cheat_init = true;
}

void cheat_unload( )
{
  g_listener_entity->remove_entities( );
  g_event_listener->remove_events( );

#ifndef _DEBUG
  g_thread_pool->destroy_threads( );
#endif

  hooks::unhook( );
}

void cheat_init( LPVOID reserved )
{
#ifdef _DEBUG
  CreateConsole( );
  printf( "Debug started. \n" );

  wait_modules( );

  cheat_load( );

  interfaces::engine->execute_cmd_unrestricted( ( "cl_fullupdate" ) );

  while( !g_ctx.uninject )
    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

  cheat_unload( );

  DestroyConsole( );

  FreeLibraryAndExitThread( cheat_module, 0 );
#else

#ifdef DEBUG_LOG
  file_stream.open( xor_c( "airflow_inject.txt" ), std::ios::binary );

  char cur_time [ 128 ]{ };

  time_t t;
  struct tm* ptm;

  t = time( NULL );
  ptm = localtime( &t );

  strftime( cur_time, 128, xor_c( "%c" ), ptm );

  file_stream << cur_time << std::endl;

  file_stream << xor_c( "START" ) << std::endl;
#endif

  wait_modules( );

#ifdef DEBUG_LOG
  file_stream << xor_c( "INIT-S" ) << std::endl;
#endif
  cheat_load( );

#ifdef DEBUG_LOG
  file_stream.close( );
#endif

#endif
}

BOOL APIENTRY DllMain( HMODULE module, uintptr_t reason, LPVOID reserved )
{
  if( reason == DLL_PROCESS_ATTACH )
  {
    cheat_module = module;

    CreateThread( 0, 0, ( LPTHREAD_START_ROUTINE )cheat_init, reserved, 0, 0 );
    return TRUE;
  }

  return FALSE;
}