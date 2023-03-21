#pragma once
#include <GLFW/glfw3.h>
#include <cutil/vector.hpp>

namespace vkUI {
class uiWindow;
}

namespace vkUI::IO {
using namespace Cutil;

enum class CallbackFrags : uint16_t {

};
/* glfwSetFramebufferSizeCallback(window, resizeCB_static); */
/* glfwSetCursorPosCallback(window, mouseCB_static); */
/* glfwSetKeyCallback(window, keyboardCB_static); */
/* glfwSetScrollCallback(window, scrollCB_static); */
/* glfwSetCharCallback(window, charCB_static); */
/* glfwSetMouseButtonCallback(window, mouseButtonCB_static); */

enum class uiCallbackFlags : uint16_t {
  OnHover = 0,      // Widget上にマウスがHoverされたとき
  OffHover = 1,     // Widget上のRectからマウスが離れた時（
  ValueChange = 2,  // 値が変更されたとき（Userによって？）
  ResizeTo = 3,     //
  MouseMove = 4,    // マウスが移動した時（HoverしているWidgetとFocusされたWidgetに送信される）
  LMouseDown = 5,   // Left mouse down
  RMouseDown = 6,   // Right mouse down
  CMouseDown = 7,   // Center mouse down
  LMouseUP = 8,     // Left mouse up
  RMouseUP = 9,     // Right mouse up
  CMouseUP = 10,    // Center mouse up
  MouseScroll = 11, // マウスがスクロールされた（スクロール量はnumに入っている）
  DragDrop = 12,    // ファイルがD＆Dされた（numにファイル数、const char **stringsにファイル名のリスト）
  ShouldClose = 13, // ウィンドウが閉じられる直前 glfwShouldWindowCloseがtrueになった時
  Keyboard = 14,    // Keyboard input
  CharInput = 15    // char input (keyboard?)
};

using InputTextFlags = uint64_t;
using InputTextFlags = uint64_t;
using uiKey = int64_t;
using uiWchar = unsigned short;

// Shared state of InputText(), passed as an argument to your callback when a ImGuiInputTextFlags_Callback* flag is used.
// The callback function should return 0 by default.
// Callbacks (follow a flag name and see comments in ImGuiInputTextFlags_ declarations for more details)
// - ImGuiInputTextFlags_CallbackEdit:        Callback on buffer edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus
// is active)
// - ImGuiInputTextFlags_CallbackAlways:      Callback on each iteration
// - ImGuiInputTextFlags_CallbackCompletion:  Callback on pressing TAB
// - ImGuiInputTextFlags_CallbackHistory:     Callback on pressing Up/Down arrows
// - ImGuiInputTextFlags_CallbackCharFilter:  Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
// - ImGuiInputTextFlags_CallbackResize:      Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow.
struct ImGuiInputTextCallbackData {
  InputTextFlags EventFlag; // One ImGuiInputTextFlags_Callback*    // Read-only
  InputTextFlags Flags;     // What user passed to InputText()      // Read-only
  void* UserData;           // What user passed to InputText()      // Read-only

  // Arguments for the different callback events
  // - To modify the text buffer in a callback, prefer using the InsertChars() / DeleteChars() function. InsertChars() will take care of calling the resize callback if necessary.
  // - If you know your edits are not going to resize the underlying buffer allocation, you may modify the contents of 'Buf[]' directly. You need to update 'BufTextLen' accordingly (0 <= BufTextLen <
  // BufSize) and set 'BufDirty'' to true so InputText can update its internal state.
  unsigned short
    EventChar;    // Character input                      // Read-write   // [CharFilter] Replace character with another one, or set to zero to drop. return 1 is equivalent to setting EventChar=0;
  uiKey EventKey; // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
  char* Buf;      // Text buffer                          // Read-write   // [Resize] Can replace pointer / [Completion,History,Always] Only write to pointed data, don't replace the actual pointer!
  int BufTextLen; // Text length (in bytes)               // Read-write   // [Resize,Completion,History,Always] Exclude zero-terminator storage. In C land: == strlen(some_text), in C++ land:
                  // string.length()
  int BufSize;    // Buffer size (in bytes) = capacity+1  // Read-only    // [Resize,Completion,History,Always] Include zero-terminator storage. In C land == ARRAYSIZE(my_char_array), in C++ land:
                  // string.capacity()+1
  bool BufDirty;  // Set if you modify Buf/BufTextLen!    // Write        // [Completion,History,Always]
  int CursorPos;  //                                      // Read-write   // [Completion,History,Always]
  int SelectionStart; //                                      // Read-write   // [Completion,History,Always] == to SelectionEnd when no selection)
  int SelectionEnd;   //                                      // Read-write   // [Completion,History,Always]

  // Helper functions for text manipulation.
  // Use those function to benefit from the CallbackResize behaviors. Calling those function reset the selection.
  ImGuiInputTextCallbackData();
  void DeleteChars(int pos, int bytes_count);
  void InsertChars(int pos, const char* text, const char* text_end = NULL);
  void SelectAll() {
    SelectionStart = 0;
    SelectionEnd = BufTextLen;
  }
  void ClearSelection() { SelectionStart = SelectionEnd = BufTextLen; }
  bool HasSelection() const { return SelectionStart != SelectionEnd; }
};

// Resizing callback data to apply custom constraint. As enabled by SetNextWindowSizeConstraints(). Callback is called during the next Begin().
// NB: For basic min/max size constraint on each axis you don't need to use the callback! The SetNextWindowSizeConstraints() parameters are enough.
struct ImGuiSizeCallbackData {
  void* UserData;       // Read-only.   What user passed to SetNextWindowSizeConstraints()
  Vector2d Pos;         // Read-only.   Window position, for reference.
  Vector2d CurrentSize; // Read-only.   Current window size.
  Vector2d DesiredSize; // Read-write.  Desired size, based on user's mouse position. Write to this field to restrain resizing.
};

// Data payload for Drag and Drop operations: AcceptDragDropPayload(), GetDragDropPayload()
struct ImGuiPayload {
  // Members
  void* Data;   // Data (copied and owned by dear imgui)
  int DataSize; // Data size

  // [Internal]
  int DataFrameCount;    // Data timestamp
  char DataType[32 + 1]; // Data type tag (short user-supplied string, 32 characters max)
  bool Preview;          // Set when AcceptDragDropPayload() was called and mouse has been hovering the target item (nb: handle overlapping drag targets)
  bool Delivery;         // Set when AcceptDragDropPayload() was called and mouse button is released over the target item.

  ImGuiPayload() { Clear(); }
  void Clear() {
    Data = NULL;
    DataSize = 0;
    memset(DataType, 0, sizeof(DataType));
    DataFrameCount = -1;
    Preview = Delivery = false;
  }
  bool IsDataType(const char* type) const { return DataFrameCount != -1 && strcmp(type, DataType) == 0; }
  bool IsPreview() const { return Preview; }
  bool IsDelivery() const { return Delivery; }
};


enum ImGuiMouseButton_ { ImGuiMouseButton_Left = 0, ImGuiMouseButton_Right = 1, ImGuiMouseButton_Middle = 2, ImGuiMouseButton_COUNT = 5 };

struct uiKeyData {
  bool down;        /// key is pressed
  float duration;   /// key down duration in milliseconds (0.0 = just pressed, duration < 0 = not pressed)
  uint8_t pressure; /// joypad press pressure
};

struct uiMouseData {
  bool clicked;        /// true if currently clicked
  bool double_clicked; /// true if currently double-clicked
  uint16_t duration;   /// duration of clicking
};

struct JoypadData {
  bool select : 1;      //!< セレクトボタン。
  bool l3 : 1;          //!< ハンドルスイッチ左。アナログもーどの時のみ有効。それ以外の時は常に1。
  bool r3 : 1;          //!< ハンドルスイッチ右。アナログモードの時のみ有効。それ以外の時は常に1。
  bool start : 1;       //!< スタートボタン。
  bool up : 1;          //!< 十字キー上。
  bool right : 1;       //!< 十字キー右。
  bool down : 1;        //!< 十字キー下。
  bool left : 1;        //!< 十字キー左。
  bool l2 : 1;          //!< L2。
  bool r2 : 1;          //!< R2。
  bool l1 : 1;          //!< L1。
  bool r1 : 1;          //!< R1。
  bool Y : 1;           //!< さんかくボタン。
  bool B : 1;           //!< まるボタン。
  bool A : 1;           //!< ばつボタン。
  bool X : 1;           //!< しかくボタン。
  std::int8_t rstick_y; //!< 右ハンドル左右方向。アナログモード時のみ有効。倒さない状況で0x80付近、上に倒しきると0x00、下に倒しきると0xFFとなる。
  std::int8_t rstick_x; //!< 右ハンドル上下方向。アナログモード時のみ有効。倒さない状況で0x80付近、左に倒しきると0x00、右に倒しきると0xFFとなる。
  std::int8_t lstick_y; //!< 左ハンドル左右方向。
  std::int8_t lstick_x; //!< 左ハンドル上下方向。
};

#define IMGUI_KEY_COUNT 512
#define IMGUI_KEYDATA_SIZE 10
class vkIO {
private:
  GLFWwindow* glfw_wnd;
  uiWindow* wnd;

  bool key_input_enabled;
  bool mouse_input_enabled;
  bool drop_event_enabled;
  bool resize_event_enabled;

  void setup();

public:
  struct vkIOConfig {
    float IniSavingRate;      // = 5.0f               // Maximum time between saving positions/sizes to .ini file, in seconds.
    float DoubleClickMaxDist; // = 6.0f               // Distance threshold to stay in to validate a double-click, in pixels.
    float DoubleClickMaxTime; ///  = 0.30f              // Time for a double-click, in seconds.
    float MouseDragThreshold; // = 6.0f               // Distance threshold before considering we are dragging

    float FontGlobalScale;     // = 1.0f               // Global scale all fonts
    bool FontAllowUserScaling; // = false              // Allow user scaling text of individual window with CTRL+Wheel.
                               //
    const char* IniFilename;   // = "imgui.ini"        // Path to .ini file. NULL to disable .ini saving.
    const char* LogFilename;   // = "imgui_log.txt"    // Path to .log file (default parameter to ImGui::LogToFile when no file is specified).

    float KeyRepeatDelay; // = 0.250f             // When holding a key/button, time before it starts repeating, in seconds (for buttons in Repeat mode, etc.).
    float KeyRepeatRate;  // = 0.020f             // When holding a key/button, rate at which it repeats, in seconds.
                          //
  };
  vkIOConfig config;

  // --------   window formats -------

  Vector2d wnd_size{-1, -1};     /// window size in pixels
  Vector2d wnd_pos{-1, -1};      /// window position in monitor (top-left = (0, 0))
  uint16_t time_from_last_frame; /// time from last frame (in milliseconds)
  uint16_t time_from_start;      /// time from window creation (in milliseconds)
  bool resized_dirty;            /// if resized set True, User must set to false to check next resize

  // --------   mouse -------

  uiMouseData mouse_l, mouse_r, mouse_m, mouse_ex1, mouse_ex2;
  Vector2d mouse_wheel;
  Vector2d mouse_pos, last_clicked_pos, mouse_pos_prev;

  JoypadData joypad;

  uiKeyData keys[IMGUI_KEYDATA_SIZE]; /// Key state for all keys. Use IsKeyXXX() functions to access this.
  bool alt_pressed, ctrl_pressed, enter_pressed;

  // --------   mouse -------

  const char* (*GetClipboardTextFn)(void* user_data);
  void (*SetClipboardTextFn)(void* user_data, const char* text);
  void* ClipboardUserData;

  /* float PenPressure;                      // Touch/Pen pressure (0.0f to 1.0f, should be >0.0f only when MouseDown[0] == true). Helper storage currently unused by Dear ImGui.
    bool
      WantCaptureMouseUnlessPopupClose; // Alternative to WantCaptureMouse: (WantCaptureMouse == true && WantCaptureMouseUnlessPopupClose == false) when a click over void is expected to close a popup.
    uint16_t MouseClickedCount[5];      // == 0 (not clicked), == 1 (same as MouseClicked[]), == 2 (double-clicked), == 3 (triple-clicked) etc. when going from !Down to Down
    uint16_t MouseClickedLastCount[5];  // Count successive number of clicks. Stays valid after mouse release. Reset after another click is done.
                                        // bounds.
    bool MouseDownOwnedUnlessPopupClose[5]; // Track if button was clicked inside a dear imgui window.
    bool AppFocusLost;                      // Only modify via AddFocusEvent()

    uiWchar InputCharacters[16 + 1]; // List of characters input (translated by user from keypress+keyboard state). Fill using AddInputCharacter() helper.

    void AddInputCharacter(uiWchar c);                             // Helper to add a new character into InputCharacters[]
    void AddInputCharactersUTF8(const char* utf8_chars);           // Helper to add new characters into InputCharacters[] from an UTF-8 string
    inline void ClearInputCharacters() { InputCharacters[0] = 0; } // Helper to clear the text input buffer

    bool WantCaptureMouse;     // Mouse is hovering a window or widget is active (= ImGui will use your mouse input)
    bool WantCaptureKeyboard;  // Widget is active (= ImGui will use your keyboard input)
    bool WantTextInput;        // Some text input widget is active, which will read input characters from the InputCharacters array.
    float Framerate;           // Framerate estimation, in frame per second. Rolling average estimation based on IO.DeltaTime over 120 frames
    int MetricsAllocs;         // Number of active memory allocations
    int MetricsRenderVertices; // Vertices output during last call to Render()
    int MetricsRenderIndices;  // Indices output during last call to Render() = number of triangles * 3
    int MetricsActiveWindows;  // Number of visible windows (exclude child windows)
  */

  vkIO();
  ~vkIO();
  void set_window(GLFWwindow* w);
  void enable_keyinput(bool);
  void enable_mouse_input(bool);
  void enable_resize_input(bool);
  void enable_drop_input(bool);

  void add_input_char(uiWchar c);
  void add_input_char_utf8(const char* utf8);
  void clear_input_char();

  void AddKeyEvent(int key, bool down); // Queue a new key down/up event. Key should be "translated" (as in, generally ImGuiKey_A matches the key end-user would use to emit an 'A' character)
  void AddKeyAnalogEvent(int key, bool down, float v); // Queue a new key down/up event for analog values (e.g. ImGuiKey_Gamepad_ values). Dead-zones should be handled by the backend.
  void AddMousePosEvent(float x, float y);                  // Queue a mouse position update. Use -FLT_MAX,-FLT_MAX to signify no mouse (e.g. app not focused and not hovered)
  void AddMouseButtonEvent(int button, bool down);          // Queue a mouse button change
  void AddMouseWheelEvent(float wh_x, float wh_y);          // Queue a mouse wheel update
  void AddFocusEvent(bool focused);                         // Queue a gain/loss of focus for the application (generally based on OS/platform focus of your window)
  void AddInputCharacter(unsigned int c);                   // Queue a new character input
  void AddInputCharacterUTF16(uiWchar *c);                 // Queue a new character input from an UTF-16 character, it can be a surrogate
  void AddInputCharactersUTF8(const char* str);             // Queue a new characters input from an UTF-8 string

  static void resizeCB_static(GLFWwindow*, int, int);
  static void mouseCB_static(GLFWwindow*, double, double);
  static void keyboardCB_static(GLFWwindow*, int, int, int, int);
  static void scrollCB_static(GLFWwindow*, double, double);
  static void charCB_static(GLFWwindow*, unsigned int);
  static void mouseButtonCB_static(GLFWwindow*, int, int, int);
#if  0
static void vkIO::resizeCB_static(GLFWwindow* window, int width, int height) {
  auto app = reinterpret_cast<vkIO*>(glfwGetWindowUserPointer(window));
  app->resizeCB(width, height);
}
static void vkIO::mouseCB_static(GLFWwindow* window, double xpos, double ypos) {
  auto app = reinterpret_cast<vkIO*>(glfwGetWindowUserPointer(window));
  app->mouseCB(xpos, ypos);
}
static void vkIO::keyboardCB_static(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto app = reinterpret_cast<vkIO*>(glfwGetWindowUserPointer(window));
  app->keyboardCB(key, scancode, action, mods);
}
static void vkIO::scrollCB_static(GLFWwindow* window, double x, double y) {
  auto app = reinterpret_cast<vkIO*>(glfwGetWindowUserPointer(window));
  app->scrollCB(x, y);
}
static void vkIO::charCB_static(GLFWwindow* window, unsigned int codepoint) {
  auto app = reinterpret_cast<vkIO*>(glfwGetWindowUserPointer(window));
  app->charCB(codepoint);
}
static void vkIO::mouseButtonCB_static(GLFWwindow* window, int button, int action, int mods) {
  auto app = reinterpret_cast<vkIO*>(glfwGetWindowUserPointer(window));
  app->mouseBtnCB(button, action, mods);
}
#endif

  void resizeCB(int w, int h);
  void mouseCB(double x, double y);
  void charCB(unsigned int codepoint);
  void keyboardCB(int key, int scancode, int action, int mods);
  void scrollCB(double xoffset, double yoffset);
  void mouseBtnCB(int button, int action, int mods);
};


} // namespace vkUI::IO
