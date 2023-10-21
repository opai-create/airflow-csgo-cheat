#include "../menu.h"
using namespace ImGui;

bool c_menu::key_bind( const char* label, key_binds_t& var )
{
  float alpha = this->get_alpha( );

  const ImVec2& size_arg = ImVec2( 256, 48 );

  var.key = std::clamp( var.key, -1, 255 );

  ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f, 0.5f * alpha ) );

  ImGuiWindow* window = ImGui::GetCurrentWindow( );
  if( window->SkipItems )
    return false;

  ImGuiContext& g = *GImGui;
  ImGuiIO& io = g.IO;
  const ImGuiStyle& style = g.Style;

  const ImGuiID id = window->GetID( label );
  bool popup_open = IsPopupOpen( id );
  const ImVec2 label_size = ImGui::CalcTextSize( label, NULL, true );

  const ImVec2 render_frame_size = size_arg;

  ImVec2 frame_size = ( size_arg / ImVec2( 3.f, 1.f ) ) - ImVec2( 14.f, 0.f );

  const auto pos = window->DC.CursorPos;
  auto cursor_pos = pos + ImVec2( size_arg - frame_size );

  const ImRect frame_bb( cursor_pos + ImVec2( 0.f, 9.f ), cursor_pos + frame_size - ImVec2( 9.f, 9.f ) );
  const ImRect total_bb( frame_bb.Min, frame_bb.Max + ImVec2( 0.f, 22.f ) );

  ImGuiWindow* draw_window = window;
  ItemSize( total_bb, style.FramePadding.y );
  if( !ItemAdd( total_bb, id, &frame_bb ) )
    return false;

  const bool focus_requested = ImGui::FocusableItemRegister( window, g.ActiveId == id );
  const bool focus_requested_by_code = focus_requested && ( window->DC.FocusCounterAll == window->DC.FocusCounterTab );
  const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;

  const bool hovered = ImGui::ItemHoverable( frame_bb, id );

  if( hovered )
  {
    ImGui::SetHoveredID( id );
    g.MouseCursor = ImGuiMouseCursor_TextInput;
  }

  const bool user_clicked = hovered && io.MouseClicked [ 0 ];

  if( focus_requested || user_clicked )
  {
    if( g.ActiveId != id )
    {
      // Start edition
      memset( io.MouseDown, 0, sizeof( io.MouseDown ) );
      memset( io.KeysDown, 0, sizeof( io.KeysDown ) );
      var.key = 0;
    }
    ImGui::SetActiveID( id, window );
    ImGui::FocusWindow( window );
  }
  else if( io.MouseClicked [ 0 ] )
  {
    // Release focus when we click outside
    if( g.ActiveId == id )
      ImGui::ClearActiveID( );
  }

  bool value_changed = false;
  int key = var.key;

  if( g.ActiveId == id )
  {
    for( auto i = 0; i < 5; i++ )
    {
      if( io.MouseDown [ i ] )
      {
        switch( i )
        {
        case 0:
          key = VK_LBUTTON;
          break;
        case 1:
          key = VK_RBUTTON;
          break;
        case 2:
          key = VK_MBUTTON;
          break;
        case 3:
          key = VK_XBUTTON1;
          break;
        case 4:
          key = VK_XBUTTON2;
          break;
        }
        value_changed = true;
        ImGui::ClearActiveID( );
      }
    }
    if( !value_changed )
    {
      for( auto i = VK_BACK; i <= VK_RMENU; i++ )
      {
        if( io.KeysDown [ i ] )
        {
          key = i;
          value_changed = true;
          ImGui::ClearActiveID( );
        }
      }
    }

    if( IsKeyPressedMap( ImGuiKey_Escape ) )
    {
      var.key = -1;
      ImGui::ClearActiveID( );
    }
    else
    {
      var.key = key;
    }
  }

  const auto ctx = GetCurrentContext( );

  if( hovered && io.MouseClicked [ 1 ] )
  {
    OpenPopupEx( id );
    popup_open = true;
  }

  auto& popup_mod = item_animations [ _fnva1( label ) + _fnva1( "popup" ) ];
  this->create_animation( popup_mod.alpha, popup_open, 0.6f, skip_disable | lerp_animation );

  if( popup_open )
  {
    // 3 bind mods
    float max_size = calc_max_popup_height( 3 );

    auto items_size = ImVec2( 63.f, max_size * popup_mod.alpha );
    SetNextWindowSize( items_size );
    SetNextWindowPos( frame_bb.Min + ImVec2( 0.f, 31.f ) );
    SetNextWindowBgAlpha( alpha * popup_mod.alpha );

    // We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;

    char name [ 16 ];
    ImFormatString( name, IM_ARRAYSIZE( name ), xor_c( "##Combo_%02d" ), ctx->BeginPopupStack.Size ); // Recycle windows based on depth

    // Horizontally align ourselves with the framed text
    PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 7, style.WindowPadding.y ) );
    bool ret = Begin( name, NULL, window_flags );
    PopStyleVar( );
    if( !ret )
    {
      EndPopup( );
      IM_ASSERT( 0 ); // This should never happen as we tested for IsPopupOpen() above
      return false;
    }

    if( this->selectable( xor_c( "Always" ), var.type == 0 ) )
      var.type = 0;
    if( this->selectable( xor_c( "Hold" ), var.type == 1 ) )
      var.type = 1;
    if( this->selectable( xor_c( "Toggle" ), var.type == 2 ) )
      var.type = 2;

    EndPopup( );
  }

  std::string buffer{ xor_c( "None" ) };
  buffer.resize( 128 );

  auto& mod = item_animations [ _fnva1( label ) ];

  const bool hovered_frame = ItemHoverable( ImRect( pos, pos + size_arg ), id );
  this->create_animation( mod.hovered_alpha, hovered_frame, 1.f, lerp_animation );

  auto& mod_frame = item_animations [ _fnva1( label ) + _fnva1( "frame" ) ];
  this->create_animation( mod_frame.hovered_alpha, hovered || g.ActiveId == id, 1.f, lerp_animation );

  // background
  auto back_alpha = ( 20 + ( 30 * mod.hovered_alpha ) ) * alpha;
  color back_clr = color( 217, 217, 217 ).increase( 38 * mod.hovered_alpha ).new_alpha( back_alpha );
  draw_list->AddRectFilled( pos, pos + size_arg, back_clr.as_imcolor( ), 4.f );

  // popup
  if( popup_open )
  {
    draw_list->AddRectFilled( frame_bb.Min, frame_bb.Max, color( 5, 5, 5, 150 * alpha * popup_mod.alpha ).u32( ), 4.f, ImDrawCornerFlags_Top );
  }
  else
  {
    // frame
    auto frame_back_alpha = ( 80 + 30 * mod_frame.hovered_alpha ) * alpha;
    color frame_clr = color( 0, 0, 0 ).increase( 38 * mod_frame.hovered_alpha ).new_alpha( frame_back_alpha );
    draw_list->AddRectFilled( frame_bb.Min, frame_bb.Max, frame_clr.as_imcolor( ), 4.f );
  }

  // combo background (separator)
  if( popup_open )
  {
    draw_list->AddLine( ImVec2( frame_bb.Min.x - 1, frame_bb.Max.y ), ImVec2( frame_bb.Min.x + 62.f, frame_bb.Max.y ), color( 255, 255, 255, 12.75f * alpha * popup_mod.alpha ).u32( ), 1.f );
  }

  if( var.type == 0 )
    buffer = xor_c( "Always" );
  else if( var.key != -1 && g.ActiveId != id )
    buffer = key_strings [ var.key ];
  else if( g.ActiveId == id )
    buffer = xor_c( "..." );

  ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f, alpha ) );
  RenderTextClipped( frame_bb.Min, frame_bb.Max, buffer.c_str( ), NULL, NULL, style.ButtonTextAlign, &frame_bb );
  ImGui::PopStyleColor( );

  if( label_size.x > 0 )
    RenderText( ImVec2( pos.x + 12, pos.y + 16 ), label );

  ImGui::PopStyleColor( );
  return value_changed;
}