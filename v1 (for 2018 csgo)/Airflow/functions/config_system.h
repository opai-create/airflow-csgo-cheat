#pragma once
#include <string>

namespace config
{
  extern std::string sounds_folder;

  void create_config_folder( );

  void create( const std::string& config_name );
  void erase( const std::string& config_name );

  void save( const std::string& config_name );
  void load( const std::string& config_name );
}