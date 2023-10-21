#pragma once
class vector3d;
struct matrix3x4_t;

class c_debug_overlay
{
public:
  virtual void add_entity_text_overlay( int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ... ) = 0;
  virtual void add_box_overlay( const vector3d& origin, const vector3d& mins, const vector3d& max, vector3d const& orientation, int r, int g, int b, int a, float duration ) = 0;
  virtual void add_sphere_overlay( const vector3d& vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration ) = 0;
  virtual void add_triangle_overlay( const vector3d& p1, const vector3d& p2, const vector3d& p3, int r, int g, int b, int a, bool noDepthTest, float duration ) = 0;
  virtual void add_line_overlay( const vector3d& origin, const vector3d& dest, int r, int g, int b, bool noDepthTest, float duration ) = 0;
  virtual void add_text_overlay( const vector3d& origin, float duration, const char* format, ... ) = 0;
  virtual void add_text_overlay( const vector3d& origin, int line_offset, float duration, const char* format, ... ) = 0;
  virtual void add_screen_text_overlay( float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text ) = 0;
  virtual void add_swept_box_overlay( const vector3d& start, const vector3d& end, const vector3d& mins, const vector3d& max, const vector3d& angles, int r, int g, int b, int a, float flDuration ) = 0;
  virtual void add_grid_overlay( const vector3d& origin ) = 0;
  virtual void add_coord_flame_overlay( const matrix3x4_t& frame, float flScale, int vColorTable [ 3 ][ 3 ] = 0 ) = 0;
  virtual int screen_position( const vector3d& point, vector3d& screen ) = 0;
  virtual int screen_position( float flXPos, float flYPos, vector3d& screen ) = 0;
  virtual void* get_first( void ) = 0;
  virtual void* get_next( void* current ) = 0;
  virtual void clear_dead_overlays( void ) = 0;
  virtual void clear_all_overlays( void ) = 0;
  virtual void add_text_overlay_rgb( const vector3d& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ... ) = 0;
  virtual void add_text_overlay_rgb( const vector3d& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ... ) = 0;
  virtual void add_line_overlay_alpha( const vector3d& origin, const vector3d& dest, int r, int g, int b, int a, bool noDepthTest, float duration ) = 0;
  virtual void add_box_overlay_alt( const vector3d& origin, const vector3d& mins, const vector3d& max, vector3d const& orientation, const color faceColor, const color edgeColor, float duration ) = 0;
  virtual void add_line_overlay( const vector3d& origin, const vector3d& dest, int r, int g, int b, int a, float, float ) = 0;
  virtual void purge_text_overlays( ) = 0;
  virtual void add_capsule_overlay( const vector3d& mins, const vector3d& max, float& radius, int r, int g, int b, int a, float duration, char unknown, char ignorez ) = 0;
};