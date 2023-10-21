#pragma once
#include "../tools/math.h"
#include "../tools/memory/memory.h"

#include <algorithm>

enum flow_type_t
{
  flow_outgoing,
  flow_incoming,
  max_flows,
};

class c_radar_info
{
public:
  vector3d pos{ };
  vector3d angle{ };
  vector3d spotted_map_angle_related{ };

  DWORD tab_related{ };

  padding( 0xC );

  float spotted_time{ };
  float spotted_fraction{ };
  float time{ };

  padding( 0x4 );

  int player_index{ };
  int entity_index{ };

  padding( 0x4 );

  int health{ };
  char name [ 32 ]{ };

  padding( 0x75 );

  bool spotted{ };

  padding( 0x8A );
};

class c_hud_radar
{
public:
  padding( 0x14C );

  c_radar_info radar_info [ 65 ]{ };
};

class i_net_channel_info
{
public:
  enum
  {
    generic = 0,  // must be first and is default group
    localplayer,  // bytes for local player entity update
    otherplayers, // bytes for other players update
    entities,     // all other entity bytes
    sounds,       // game sounds
    events,       // event messages
    tempents,     // temp entities
    usermessages, // user messages
    entmessages,  // entity messages
    voice,        // voice data
    stringtable,  // a stringtable update
    move,         // client move cmds
    stringcmd,    // string command
    signon,       // various signondata
    total         // must be last and is not a real group
  };

  virtual const char* get_name( void ) const = 0;     // get channel name
  virtual const char* get_address( void ) const = 0;  // get channel IP address as string
  virtual float get_time( void ) const = 0;           // current net time
  virtual float get_time_connected( void ) const = 0; // get connection time in seconds
  virtual int get_buffer_size( void ) const = 0;      // netchannel packet history size
  virtual int get_data_rate( void ) const = 0;        // send data rate in byte/sec

  virtual bool is_loopback( void ) const = 0;   // true if loopback channel
  virtual bool is_timing_out( void ) const = 0; // true if timing out
  virtual bool is_playback( void ) const = 0;   // true if demo playback

  virtual float get_latency( int flow ) const = 0;     // current latency (RTT), more accurate but jittering
  virtual float get_avg_latency( int flow ) const = 0; // average packet latency in seconds
};

class i_netchannel: public i_net_channel_info
{
};

class bf_read
{
public:
  const char* m_pDebugName;
  bool m_bOverflow;
  int m_nDataBits;
  unsigned int m_nDataBytes;
  unsigned int m_nInBufWord;
  int m_nBitsAvail;
  const unsigned int* m_pDataIn;
  const unsigned int* m_pBufferEnd;
  const unsigned int* m_pData;

  bf_read( ) = default;

  bf_read( const void* pData, int nBytes, int nBits = -1 )
  {
    StartReading( pData, nBytes, 0, nBits );
  }

  void StartReading( const void* pData, int nBytes, int iStartBit, int nBits )
  {
    // Make sure it's dword aligned and padded.
    m_pData = ( uint32_t* )pData;
    m_pDataIn = m_pData;
    m_nDataBytes = nBytes;

    if( nBits == -1 )
    {
      m_nDataBits = nBytes << 3;
    }
    else
    {
      m_nDataBits = nBits;
    }
    m_bOverflow = false;
    m_pBufferEnd = reinterpret_cast< uint32_t const* >( reinterpret_cast< uint8_t const* >( m_pData ) + nBytes );
    if( m_pData )
      Seek( iStartBit );
  }

  bool Seek( int nPosition )
  {
    bool bSucc = true;
    if( nPosition < 0 || nPosition > m_nDataBits )
    {
      m_bOverflow = true;
      bSucc = false;
      nPosition = m_nDataBits;
    }
    int nHead = m_nDataBytes & 3; // non-multiple-of-4 bytes at head of buffer. We put the "round off"
                                  // at the head to make reading and detecting the end efficient.

    int nByteOfs = nPosition / 8;
    if( ( m_nDataBytes < 4 ) || ( nHead && ( nByteOfs < nHead ) ) )
    {
      // partial first dword
      uint8_t const* pPartial = ( uint8_t const* )m_pData;
      if( m_pData )
      {
        m_nInBufWord = *( pPartial++ );
        if( nHead > 1 )
          m_nInBufWord |= ( *pPartial++ ) << 8;
        if( nHead > 2 )
          m_nInBufWord |= ( *pPartial++ ) << 16;
      }
      m_pDataIn = ( uint32_t const* )pPartial;
      m_nInBufWord >>= ( nPosition & 31 );
      m_nBitsAvail = ( nHead << 3 ) - ( nPosition & 31 );
    }
    else
    {
      int nAdjPosition = nPosition - ( nHead << 3 );
      m_pDataIn = reinterpret_cast< uint32_t const* >( reinterpret_cast< uint8_t const* >( m_pData ) + ( ( nAdjPosition / 32 ) << 2 ) + nHead );
      if( m_pData )
      {
        m_nBitsAvail = 32;
        GrabNextDWord( );
      }
      else
      {
        m_nInBufWord = 0;
        m_nBitsAvail = 1;
      }
      m_nInBufWord >>= ( nAdjPosition & 31 );
      m_nBitsAvail = std::min< int >( m_nBitsAvail, 32 - ( nAdjPosition & 31 ) ); // in case grabnextdword overflowed
    }
    return bSucc;
  }

  FORCEINLINE void GrabNextDWord( bool bOverFlowImmediately = false )
  {
    if( m_pDataIn == m_pBufferEnd )
    {
      m_nBitsAvail = 1; // so that next read will run out of words
      m_nInBufWord = 0;
      m_pDataIn++; // so seek count increments like old
      if( bOverFlowImmediately )
        m_bOverflow = true;
    }
    else if( m_pDataIn > m_pBufferEnd )
    {
      m_bOverflow = true;
      m_nInBufWord = 0;
    }
    else
    {
      m_nInBufWord = *( m_pDataIn++ );
    }
  }
};

class bf_write
{
public:
  unsigned char* m_pData;
  int m_nDataBytes;
  int m_nDataBits;
  int m_iCurBit;
  bool m_bOverflow;
  bool m_bAssertOnOverflow;
  const char* m_pDebugName;

  void StartWriting( void* pData, int nBytes, int iStartBit = 0, int nBits = -1 )
  {
    // Make sure it's dword aligned and padded.
    // The writing code will overrun the end of the buffer if it isn't dword aligned, so truncate to force alignment
    nBytes &= ~3;

    m_pData = ( unsigned char* )pData;
    m_nDataBytes = nBytes;

    if( nBits == -1 )
    {
      m_nDataBits = nBytes << 3;
    }
    else
    {
      m_nDataBits = nBits;
    }

    m_iCurBit = iStartBit;
    m_bOverflow = false;
  }

  bf_write( )
  {
    m_pData = NULL;
    m_nDataBytes = 0;
    m_nDataBits = -1; // set to -1 so we generate overflow on any operation
    m_iCurBit = 0;
    m_bOverflow = false;
    m_bAssertOnOverflow = true;
    m_pDebugName = NULL;
  }

  // nMaxBits can be used as the number of bits in the buffer.
  // It must be <= nBytes*8. If you leave it at -1, then it's set to nBytes * 8.
  bf_write( void* pData, int nBytes, int nBits = -1 )
  {
    m_bAssertOnOverflow = true;
    m_pDebugName = NULL;
    StartWriting( pData, nBytes, 0, nBits );
  }

  bf_write( const char* pDebugName, void* pData, int nBytes, int nBits = -1 )
  {
    m_bAssertOnOverflow = true;
    m_pDebugName = pDebugName;
    StartWriting( pData, nBytes, 0, nBits );
  }
};

class c_net_message
{
public:
  virtual ~c_net_message( ){ };

  virtual void set_net_channel( void* netchan ) = 0; // netchannel this message is from/for
  virtual void set_reliable( bool state ) = 0;       // set to true if it's a reliable message

  virtual bool process( void ) = 0; // calles the recently set handler to process this message

  virtual bool read_from_buffer( bf_read& buffer ) = 0; // returns true if parsing was OK
  virtual bool write_to_buffer( bf_write& buffer ) = 0; // returns true if writing was OK

  virtual bool is_reliable( void ) const = 0; // true, if message needs reliable handling

  virtual int get_type( void ) const = 0;         // returns module specific header tag eg svc_serverinfo
  virtual int get_group( void ) const = 0;        // returns net message group of this message
  virtual const char* get_name( void ) const = 0; // returns network message name, eg "svc_serverinfo"
  virtual i_netchannel* get_net_channel( void ) const = 0;
  virtual const char* to_string( void ) const = 0; // returns a human readable string about message content
};

class clc_move
{
private:
  padding( 0x8 );

public:
  int backup_commands;
  int new_commands;
  void* side;
  int _cached_size_;
  uint32_t _has_bits_ [ ( 3 + 31 ) / 32 ];
};

template < typename T >
class c_netmessagepb: public c_net_message, public T
{
};
using clc_move_t = c_netmessagepb< clc_move >;

class c_netchan
{
public:
  padding( 0x18 );

  int out_sequence_nr{ };
  int in_sequence_nr{ };
  int out_sequence_nr_ack{ };

  int out_reliable_state{ };
  int in_reliable_state{ };

  int choked_packets{ };

  int send_datagram( )
  {
    using fn = int( __thiscall* )( void*, void* );
    return g_memory->getvfunc< fn >( this, 48 )( this, 0 );
  }
};

class c_eventinfo
{
public:
  short class_id{ };
  float fire_delay{ };
  const void* send_table{ };
  const void* client_class{ };
  int bits{ };
  uint8_t* data{ };
  int flags{ };

  padding( 0x1C );

  c_eventinfo* next{ };
};

class c_clientstate
{
public:
  class c_clock_drift_manager
  {
  public:
    float clock_offsets [ 0x10 ]{ };
    uint32_t cur_clock_offset{ };
    uint32_t server_tick{ };
    uint32_t client_tick{ };
  };

  padding( 0x9C );

  c_netchan* net_channel_ptr{ };
  int challenge_nr{ };

  padding( 0x4 );

  double connect_time{ };
  int retry_number{ };

  padding( 0x54 );

  int signon_state{ };

  padding( 0x4 );

  double next_cmd_time{ };
  int server_count{ };
  int current_sequence{ };

  padding( 0x8 );

  c_clock_drift_manager clock_drift_mgr{ };

  int delta_tick{ };

  padding( 19104 );

  int max_clients{ };

  padding( 140 );

  float frame_time{ };
  int last_outgoing_command{ };
  int choked_commands{ };
  int last_command_ack{ };

  padding( 308 );

  c_eventinfo* events{ };
};

struct player_info_t
{
  int64_t unknown{ };

  union
  {
    int64_t steaid64;
    struct
    {
      int32_t xuid_low;
      int32_t xuid_high;
    };
  };

  char name [ 128 ]{ };
  int user_id{ };
  char sz_steaid [ 20 ]{ };

  padding( 0x10 );

  unsigned long steaid{ };
  char friends_name [ 128 ]{ };
  bool fakeplayer{ };
  bool ishltv{ };
  unsigned int customFiles [ 4 ]{ };
  unsigned char files_downloaded{ };
};

class c_engine
{
public:
  i_net_channel_info* get_net_channel_info( )
  {
    using fn = i_net_channel_info*( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 78 )( this );
  }

  void* get_bsp_tree_query( )
  {
    using fn = void*( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 43 )( this );
  }

  bool console_opened( )
  {
    using fn = bool( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 11 )( this );
  }

  int get_local_player( )
  {
    using fn = int( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 12 )( this );
  }

  int get_player_for_user_id( int user_id )
  {
    using fn = int( __thiscall* )( PVOID, int );
    return g_memory->getvfunc< fn >( this, 9 )( this, user_id );
  }

  void get_player_info( int index, player_info_t* info )
  {
    using fn = void( __thiscall* )( c_engine*, int, player_info_t* );
    return g_memory->getvfunc< fn >( this, 8 )( this, index, info );
  }

  void get_screen_size( int& width, int& height )
  {
    using fn = void( __thiscall* )( c_engine*, int&, int& );
    return g_memory->getvfunc< fn >( this, 5 )( this, width, height );
  }

  bool is_in_game( )
  {
    using fn = bool( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 26 )( this );
  }

  bool is_connected( )
  {
    using fn = bool( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 27 )( this );
  }

  bool is_playing_demo( )
  {
    using fn = bool( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 82 )( this );
  }

  bool is_hltv( )
  {
    using fn = bool( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 93 )( this );
  }

  void execute_cmd( const char* cmd )
  {
    using fn = void( __thiscall* )( c_engine*, const char* );
    return g_memory->getvfunc< fn >( this, 108 )( this, cmd );
  }

  void execute_cmd_unrestricted( const char* cmd, const char* flag = 0 )
  {
    using fn = void( __thiscall* )( c_engine*, const char*, const char* );
    return g_memory->getvfunc< fn >( this, 114 )( this, cmd, flag );
  }

  void set_view_angles( vector3d& angles )
  {
    using fn = void( __thiscall* )( c_engine*, vector3d& );
    return g_memory->getvfunc< fn >( this, 19 )( this, angles );
  }

  void get_view_angles( vector3d& angles )
  {
    using fn = void( __thiscall* )( void*, vector3d& );
    return g_memory->getvfunc< fn >( this, 18 )( this, angles );
  }

  matrix3x4_t& world_to_screen_matrix( )
  {
    using fn = matrix3x4_t&( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 37 )( this );
  }

  bool is_taking_screenshot( )
  {
    using fn = bool( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 92 )( this );
  }

  const char* get_level_name( )
  {
    using fn = const char*( __thiscall* )( c_engine* );
    return g_memory->getvfunc< fn >( this, 53 )( this );
  }

  void fire_events( )
  {
    using fn = void( __thiscall* )( c_engine* );
    g_memory->getvfunc< fn >( this, 58 )( this );
  }
};

class c_network_string_table
{
public:
  int add_string( bool is_server, const char* value, int length = -1, const void* userdata = nullptr )
  {
    using fn = int( __thiscall* )( void*, bool, const char*, int, const void* );
    return g_memory->getvfunc< fn >( this, 8 )( this, is_server, value, length, userdata );
  }
};

class c_network_string_table_container
{
public:
  c_network_string_table* find_table( const char* name )
  {
    using fn = c_network_string_table*( __thiscall* )( void*, const char* );
    return g_memory->getvfunc< fn >( this, 3 )( this, name );
  }
};