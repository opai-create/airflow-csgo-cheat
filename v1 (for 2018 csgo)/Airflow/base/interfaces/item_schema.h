#pragma once

#include <cstddef>
#include "../tools/utils_macro.h"

template < typename key, typename value >
struct node_t
{
  int prev_id{ };
  int next_id{ };
  void* unknown_ptr{ };
  int unk{ };
  key _key{ };
  value _value{ };
};

template < typename key, typename value >
struct head_t
{
  node_t< key, value >* memory{ };
  int alloc_count{ };
  int grow_siez{ };
  int start_element{ };
  int next_available{ };
  int _unknown{ };
  int last_element{ };
};

struct string_t
{
  char* buffer{ };
  int capacity{ };
  int grow_size{ };
  int length{ };
};

struct paint_kit_t
{
  int id{ };
  string_t name{ };
  string_t description{ };
  string_t item_name{ };
};

struct sticker_kit_t
{
  int id{ };

  padding( 36 );

  string_t item_name{ };
};

class c_item_schema
{
  padding( 0x28C );

public:
  head_t< int, paint_kit_t* > paint_kits{ };

private:
  padding( 0x8 );

public:
  head_t< int, sticker_kit_t* > sticker_kits{ };
};
