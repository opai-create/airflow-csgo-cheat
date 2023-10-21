#pragma once
#include "../tools/memory/memory.h"

struct model_t
{
  void* handle{ };
  char name [ MAX_PATH ]{ };

  int load_flags{ };
  int server_count{ };
  int type{ };
  int flags{ };

  vector3d mins, maxs{ };
  float radius{ };
  void* key_values{ };

  union
  {
    void* brush;
    unsigned short studio;
    void* sprite;
  };
};