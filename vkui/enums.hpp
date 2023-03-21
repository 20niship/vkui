#pragma once
#include <cutil/vector.hpp>

namespace vkUI {
using namespace Cutil;

struct uiContext;
struct uiStyle;
class uiShader;
class uiFont;
class uiWidget;

// enum class uiFrameFlags_ : uint16_t {
// 	Visible                  = 1 << 1,
// 	Active                   = 1 << 2,
// 	EnableScroll_X           = 1 << 4,   // Always show scrollbar (even if ContentSize.y < Size.y)
// 	EnableScroll_Y           = 1 << 12,   // Always show scrollbar (even if ContentSize.y < Size.y)
//     EnableMouseInputs        = 1 << 5,   // Enable mouse callback(mouse Click or mouse Move)
// 	EnableKeyboardInputs     = 1 << 6,   // Enable keyboard callback
// 	EnableChildWidget        = 1 << 7,   // Enable child window and show child windows
// 	EnableDragDropInputs     = 1 << 14,  // Enable child window and show child windows

// 	// [internal] Please access via getter/setter functions
// 	EnableAutoExpandX        = 1 << 9,   // Widgetの大きさが小さく親Widgetに余白が余っている場合、ギリギリまで拡大する
// 	EnableAutoExpandY        = 1 << 10,  // Widgetの大きさが小さく親Widgetに余白が余っている場合、ギリギリまで拡大する
// 	EnableUserResize         = 1 << 11,  // Widgetの大きさが小さく親Widgetに余白が余っている場合、ギリギリまで拡大する

// 	needRenderingAll         = 1 << 13   // 強制的にレンダリングを行う（VertexArrayとかに描画データを格納）
// 	// bool Show Srcoll = ((RectがFrame全体の大きさより大きい) && !Noscroll) || EnableScrollbarAlways
// };


enum class uiWidgetAlignTypes : uint8_t {
  Absolute,           // 絶対座標によってWidgetの一を指定します。デフォルトです。
  HorizontalList,     // Listで左から右にWidgetを格納します。同列に複数のWidgetを追加するにはFrame Widgetを追加してください
  VertialListl,       // Listで上から下にWidgetを格納します。同行に複数のWidgetを追加するにはFrame Widgetを追加してください
  Horizontel_noSpace, // HorizontalListのuiWidgetOuterSpaceを無くして、Widgetどうしをくっつけたもの
  Vertical_noSpace,   // VertialListlのuiWidgetOuterSpaceを無くして、Widgetどうしをくっつけたもの
  Grid,               // GridによってWidgetを配置します。
};


struct uiStyle {
  Vector2d WindowPadding, WidgetPadding;
  Vector2d WindowMargin, WidgetMargin;
  Vector2d WindowMinSize, WidgetMinSize;
  Vector2d WindowTitleAlign;

  uint16_t WindowBorderSize;
  uint16_t WindowMenuButtonPosition;
  uint16_t ScrollbarWidth;
  uint16_t ScrollbarRounding;
  uint16_t WidgetBorderSize;
  uint16_t IndentSpacing;
  uint16_t ColumnsMinSpacing;
  uint16_t TitlebarHeight;
  uint16_t TextSpacing;

  float MouseCursorScale;
  float FontSize;
  // std::string FontName;
  // TextAlignPos textlign;
  bool TextOverflowHidden;
  bool TextAutoSpacing;

  using colorType = Vector3b;
  colorType col_Text;
  colorType col_TextDisabled;
  colorType col_TextHovered;
  colorType col_TextNoEmphasize;
  colorType col_WndBg;
  colorType col_wndLine;
  colorType col_wndScroll;
  colorType col_wndScrollBg;
  colorType col_PopupBg;
  colorType col_PopupLine;

  colorType col_WidgetBg;
  colorType col_WidgetBgHover;
  colorType col_WidgetBgSelected;
  colorType col_WidgetBgDisabled;
  colorType col_WidgetBgMain;
  colorType col_WidgetLine;
  colorType col_WidgetLineHover;
  colorType col_WidgetLineSelected;
  colorType col_WidgetLineDisabled;

  uiStyle() { set_dark_style(); }
  void set_dark_style() {
    WindowPadding = {5, 5};
    WindowMinSize = {100, 100};
    WindowTitleAlign = {5, 5};
    WidgetPadding = {3, 3};
    WidgetMinSize = {50, 50};
    WidgetMargin = {5, 5};

    WindowBorderSize = 5;
    WindowMenuButtonPosition = 5;
    ScrollbarWidth = 7;
    ScrollbarRounding = 5;

    WidgetBorderSize = 5;
    IndentSpacing = 5;
    ColumnsMinSpacing = 5;
    MouseCursorScale = 5;

    FontSize = 11.0f;
    /* FontName="Meiryo.ttf"; */
    /* textlign = TextAlignPos::center; */
    TextOverflowHidden = true;
    TextAutoSpacing = true;
    TextSpacing = 1;

    TitlebarHeight = FontSize + WidgetPadding[1] * 2;

    col_Text = {255, 255, 255};
    col_TextDisabled = {200, 200, 200};
    col_TextHovered = {255, 255, 255};
    col_TextNoEmphasize = {255, 255, 255};
    col_WndBg = {36, 36, 36};
    col_wndLine = {150, 150, 150};
    col_wndScroll = {150, 150, 150};
    col_wndScrollBg = {80, 80, 80};
    col_PopupBg = {0, 0, 0};
    col_PopupLine = {255, 255, 255};
    col_WidgetBg = {30, 30, 50};
    col_WidgetBgHover = {82, 100, 145};
    col_WidgetBgSelected = {50, 50, 50};
    col_WidgetBgDisabled = {30, 30, 30};
    col_WidgetBgMain = {23, 23, 23};
    col_WidgetLine = {100, 100, 100};
    col_WidgetLineHover = {255, 255, 255};
    col_WidgetLineSelected = {30, 150, 150};
    col_WidgetLineDisabled = {200, 200, 200};
  }
};

enum ImGuiKey_
{
    ImGuiKey_Tab,       // for tabbing through fields
    ImGuiKey_LeftArrow, // for text edit
    ImGuiKey_RightArrow,// for text edit
    ImGuiKey_UpArrow,   // for text edit
    ImGuiKey_DownArrow, // for text edit
    ImGuiKey_PageUp,
    ImGuiKey_PageDown,
    ImGuiKey_Home,      // for text edit
    ImGuiKey_End,       // for text edit
    ImGuiKey_Delete,    // for text edit
    ImGuiKey_Backspace, // for text edit
    ImGuiKey_Enter,     // for text edit
    ImGuiKey_Escape,    // for text edit
    ImGuiKey_A,         // for text edit CTRL+A: select all
    ImGuiKey_C,         // for text edit CTRL+C: copy
    ImGuiKey_V,         // for text edit CTRL+V: paste
    ImGuiKey_X,         // for text edit CTRL+X: cut
    ImGuiKey_Y,         // for text edit CTRL+Y: redo
    ImGuiKey_Z,         // for text edit CTRL+Z: undo
    ImGuiKey_COUNT
};
} // namespace vkUI
