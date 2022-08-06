#pragma once

#ifdef WITH_OPENGL // CmakeListsをみてね
#define GLEW_STATIS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gl/internal.hpp>
using uiRenderer = vkUI::Render::glRender;
using uiWndRenderer = vkUI::Render::glWndRender;
#else
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vk/internal.hpp>
using uiRenderer = vkUI::Render::vkRender;
using uiWndRenderer = vkUI::Render::vkWndRender;
#endif

#include <cmath>
#include <limits>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <freetype/ftsynth.h>
#include <fstream>
#include <ft2build.h>
#include <iostream>
#include <numeric>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include FT_FREETYPE_H

#include <cutil/vector.hpp>
#include <drawdata.hpp>
#include <enums.hpp>
#include <stb_image.h>
#include <uiCamera.hpp>
#include <widget.hpp>


namespace vkUI{
class uiFont;
class uiWindow;
class uiWidget;

using uihWnd = uiWindow*;
using uihWidget = uiWidget*;

// keyboardCBで呼ばれる関数
// keycoard, scancode, action, mods, position
using KeyboardFuncT = std::function<bool(int, int, int, int, Vector2)>;

// -----------------------------------------------------
//    [SECTION] uiWindow
// -----------------------------------------------------
class uiWindow {
private:
  struct Cursors {
    Cursors() = default;
    void init() {
      arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
      ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
      crosshar = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
      hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
      hresize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
      vresize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    }
    GLFWcursor *arrow, *ibeam, *crosshar, *hand, *hresize, *vresize;
  } cursors;

  CameraPosition camera_position;

  Render::DrawData dd;
  uiWndRenderer renderer;

  float fps;
  uint64_t currentFrame;
  std::string name;
  Vector2d pos, size;
  ::vkUI::uiRect clipping_rect{0, 0, 99999, 999999};
  KeyboardFuncT user_key_cb;
  void updateUniformBuffer();

public:
  struct _wndStyle {
    unsigned char EnableTitleBar : 1;    // Enable title bar
    unsigned char EnableChildWindow : 1; // Enable child window and show child windows
    unsigned char EnablePopups : 1;      // Enable popup Window
    unsigned char isFullScreen : 1;
  } wndStyle;

  ::vkUI::uiRootWidget root_widget;
  ::vkUI::uiRootWidget2D root_widget_ui;
  GLFWwindow* window;
  bool framebufferResized;

  auto get_renderer() { return &renderer; }

  static void resizeCB_static(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window));
    app->resizeCB(width, height);
  }
  static void mouseCB_static(GLFWwindow* window, double xpos, double ypos) {
    auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window));
    app->mouseCB(xpos, ypos);
  }
  static void keyboardCB_static(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window));
    app->keyboardCB(key, scancode, action, mods);
  }
  static void scrollCB_static(GLFWwindow* window, double x, double y) {
    auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window));
    app->scrollCB(x, y);
  }
  static void charCB_static(GLFWwindow* window, unsigned int codepoint) {
    auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window));
    app->charCB(codepoint);
  }
  static void mouseButtonCB_static(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window));
    app->mouseBtnCB(button, action, mods);
  }

public:
  uiWindow(std::string _name, uint16_t width, uint16_t height);
  ~uiWindow();
  void drawFrame(const bool verbose = false);
  void drawDevelopperHelps();
  void updateVertexBuffer();
  void init();
  inline auto getGLFWwindow() const { return window; }
  inline bool wndShouldClose() { return glfwWindowShouldClose(window); }
  Vector2d getWindowSize() const {
    int x, y;
    glfwGetWindowSize(window, &x, &y);
    return {x, y};
  }
  Vector2d getWindowPos() const { return pos; }
#ifdef VKUI_ENGINE_ENABLE_FPS_CALC
  inline float getFPS() {
    return fps;
  }
#endif

  // cursors
  inline void setHandCursor() const {
    glfwSetCursor(window, cursors.hand);
  }
  inline void setArrowCursor() const {
    glfwSetCursor(window, cursors.arrow);
  }
  inline void setHResizeCursor() const {
    glfwSetCursor(window, cursors.hresize);
  }
  inline void setVResizeCursor() const {
    glfwSetCursor(window, cursors.vresize);
  }
  inline void setCrossHairCursor() const {
    glfwSetCursor(window, cursors.crosshar);
  }
  inline void setIbeamCursor() const {
    glfwSetCursor(window, cursors.ibeam);
  }
  inline void setDefaultCursor() const {
    glfwSetCursor(window, NULL);
  }

  inline void setUserKeyboardCB(const KeyboardFuncT& f) {
    user_key_cb = f;
  }
  const KeyboardFuncT& getUserKeyboardCB() const {
    return user_key_cb;
  }
  // --------------     rendering functions     -------------
  inline void setClippingRect(const vkUI::uiRect& r) {
    clipping_rect = r;
  }
  inline vkUI::uiRect getClippingRect() const {
    return clipping_rect;
  }

  inline void resizeCB(int w, int h) {
    framebufferResized = true;
    size[0] = w;
    size[1] = h;
    root_widget_ui.needRendering(true);
    renderer.update_wndsize(); // draw()の最後似合ったのを変更
  }

  void mouseCB(double x, double y);

  void charCB(unsigned int codepoint) {
    std::cout << codepoint << std::endl;
    const double scale_ = (camera_position.pos - camera_position.dir).norm();
    if(codepoint == 'w') {
      camera_position.dir[2] += scale_ * 0.01;
    }
    if(codepoint == 's') {
      camera_position.dir[2] -= scale_ * 0.01;
    }
    if(codepoint == 'a') {
      camera_position.dir[0] += scale_ * 0.01;
    }
    if(codepoint == 'd') {
      camera_position.dir[0] -= scale_ * 0.01;
    }
    if(codepoint == 'q') {
      camera_position.dir[1] += scale_ * 0.01;
    }
    if(codepoint == 'e') {
      camera_position.dir[1] -= scale_ * 0.01;
    }
    root_widget_ui.CallbackFunc(uiCallbackFlags::CharInput, {0, 0}, codepoint, 0, nullptr);
  }

  void keyboardCB(int key, int scancode, int action, int mods) {
    // if (key == GLFW_KEY_W && action == GLFW_PRESS){ camera_position.pos[2]+= 0.1; }
    // if (key == GLFW_KEY_S && action == GLFW_PRESS){ camera_position.pos[2]-= 0.1; }
    // if (key == GLFW_KEY_A && action == GLFW_PRESS){ camera_position.pos[0]+= 0.1; }
    // if (key == GLFW_KEY_D && action == GLFW_PRESS){ camera_position.pos[0]-= 0.1; }
    // if (key == GLFW_KEY_Q && action == GLFW_PRESS){ camera_position.pos[1]+= 0.1; }
    // if (key == GLFW_KEY_E && action == GLFW_PRESS){ camera_position.pos[1]-= 0.1; }
    root_widget_ui.CallbackFunc(uiCallbackFlags::Keyboard, {0, 0}, key, action, nullptr);
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    user_key_cb(key, scancode, action, mods, {xpos, ypos});
  }

  void scrollCB(double xoffset, double yoffset);

  void mouseBtnCB(int button, int action, [[maybe_unused]] int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if(action == GLFW_RELEASE) {
      switch(button) {
        case GLFW_MOUSE_BUTTON_RIGHT: root_widget_ui.CallbackFunc(uiCallbackFlags::RMouseUP, {(int)xpos, (int)ypos}, 0, 0, nullptr); break;
        case GLFW_MOUSE_BUTTON_LEFT: root_widget_ui.CallbackFunc(uiCallbackFlags::LMouseUP, {(int)xpos, (int)ypos}, 0, 0, nullptr); break;
        case GLFW_MOUSE_BUTTON_MIDDLE: root_widget_ui.CallbackFunc(uiCallbackFlags::CMouseUP, {(int)xpos, (int)ypos}, 0, 0, nullptr); break;
        default: MY_ASSERT("undefinded mouse button!\n");
      }
    } else {
      switch(button) {
        case GLFW_MOUSE_BUTTON_RIGHT: root_widget_ui.CallbackFunc(uiCallbackFlags::RMouseDown, {(int)xpos, (int)ypos}, 0, 0, nullptr); break;
        case GLFW_MOUSE_BUTTON_LEFT: root_widget_ui.CallbackFunc(uiCallbackFlags::LMouseDown, {(int)xpos, (int)ypos}, 0, 0, nullptr); break;
        case GLFW_MOUSE_BUTTON_MIDDLE: root_widget_ui.CallbackFunc(uiCallbackFlags::CMouseDown, {(int)xpos, (int)ypos}, 0, 0, nullptr); break;
        default: MY_ASSERT("undefinded mouse button!\n");
      }
    }
  }

  // ----------  rendering functions --------------
  inline void RemoveVerticies() {
    /* dd.vertices.resize(0); */
    dd.clear();
  }
  inline void __AddPointSizeZero(const Vector3& pos, const Vector3b& col) {
    dd.vertices.push_back(std::move(Render::Vertex(pos, col)));
  }
  [[deprecated]] inline void __AddPointSizeZero(const Vector3& pos, const Vector3b& col, [[maybe_unused]] const Vector2& uv) {
    /* dd.vertices.push_back(std::move(Vertex(pos, col))); */
  }
  inline void AddTriangle(const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3, const float width = -1) {
    if(width < 0) {
      __AddPointSizeZero(pos1, col1);
      __AddPointSizeZero(pos2, col2);
      __AddPointSizeZero(pos3, col3);
    } else {
      AddLine(pos1, pos2, col1, col2, width);
      AddLine(pos2, pos3, col2, col3, width);
      AddLine(pos3, pos1, col3, col1, width);
    }
  }
  inline void AddTriangle(const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Vector3b& col, const float width = -1) {
    AddTriangle(pos1, pos2, pos3, col, col, col, width);
  }

  /* inline void AddTriangle(const Vector3 &pos1, const Vector3 &pos2, const Vector3 &pos3, const Vector2 uv1, const Vector2 &uv2, const Vector2 &uv3, const Vector3b &col){ */
  /*   __AddPointSizeZero(pos1, col); */
  /*   __AddPointSizeZero(pos2, col); */
  /*   __AddPointSizeZero(pos3, col); */
  /* } */

  inline void AddQuad(const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Vector3& pos4, const Vector3b& col, const float width) {
    AddLine(pos1, pos2, col, width);
    AddLine(pos2, pos3, col, width);
    AddLine(pos3, pos4, col, width);
    AddLine(pos4, pos1, col, width);
  }

  inline void AddQuad(const Vector3& pos1, const Vector3& pos2, const Vector3& pos3, const Vector3& pos4, const Vector3b col) {
    AddTriangle(pos1, pos2, pos3, col);
    AddTriangle(pos1, pos3, pos4, col);
  }

  inline void AddRect(const Vector3& pos, const Vector2& size, const Vector3& normal, const Vector3b& col) {
    const auto hs = size / 2;
    const auto dp = normal.normalize();
    Vector3 e1, e2;
    if(dp[0] >= dp[1] && dp[0] >= dp[2]) { // X軸に近い直線
      e1 = {dp[1], dp[0], dp[2]};
      e2 = {dp[2], dp[1], dp[0]};
    } else if(dp[1] >= dp[0] && dp[1] >= dp[2]) {
      e1 = {dp[1], dp[0], dp[2]};
      e2 = {dp[0], dp[2], dp[1]};
    } else {
      e1 = {dp[2], dp[1], dp[0]};
      e2 = {dp[0], dp[2], dp[1]};
    }
    e1 = dp.cross(e1);
    e1 = e1.normalize();
    e2 = dp.cross(e1);
    e2 = e2.normalize();
    AddQuad(pos + Vector3{-hs[0], hs[1], 0}, pos + Vector3{-hs[0], -hs[1], 0}, pos + Vector3{hs[0], -hs[1], 0}, pos + Vector3{hs[0], hs[1], 0}, col);
  }

  // draw cyliinder surface
  inline void AddCylinder(const Vector3& p, const Vector3 n, const double r, const Vector3b& col) {
    constexpr int N = 20; // 側面分割数
    const auto dp = n.normalize();
    Vector3 e1, e2;
    if(dp[0] >= dp[1] && dp[0] >= dp[2]) { // X軸に近い直線
      e1 = {dp[1], dp[0], dp[2]};
      e2 = {dp[2], dp[1], dp[0]};
    } else if(dp[1] >= dp[0] && dp[1] >= dp[2]) {
      e1 = {dp[1], dp[0], dp[2]};
      e2 = {dp[0], dp[2], dp[1]};
    } else {
      e1 = {dp[2], dp[1], dp[0]};
      e2 = {dp[0], dp[2], dp[1]};
    }

    e1 = dp.cross(e1);
    e1 = e1.normalize();
    e2 = dp.cross(e1);
    e2 = e2.normalize();
    for(int i = 0; i < N + 1; i++) {
      const double theta1 = 6.28 * (float)i / N;
      const double theta2 = 6.28 * (float)(i + 1) / N;
      const auto v1 = (e1 * std::cos(theta1) + e2 * std::sin(theta1)) * r;
      const auto v2 = (e1 * std::cos(theta2) + e2 * std::sin(theta2)) * r;
      AddQuad(p - n / 2 + v1, p - n / 2 + v2, p + n / 2 + v2, p + n / 2 + v1, col);
      AddTriangle(p - n / 2, p - n / 2 + v2, p - n / 2 + v1, col);
      AddTriangle(p + n / 2, p + n / 2 + v1, p + n / 2 + v2, col);
    }
  }

  inline void AddCylinderLine(const Vector3& p, const Vector3 n, const double r, const Vector3b& col, const int width = 2) {
    constexpr int N = 20; // 側面分割数
    const auto dp = n.normalize();
    Vector3 e1, e2;
    if(dp[0] >= dp[1] && dp[0] >= dp[2]) { // X軸に近い直線
      e1 = {dp[1], dp[0], dp[2]};
      e2 = {dp[2], dp[1], dp[0]};
    } else if(dp[1] >= dp[0] && dp[1] >= dp[2]) {
      e1 = {dp[1], dp[0], dp[2]};
      e2 = {dp[0], dp[2], dp[1]};
    } else {
      e1 = {dp[2], dp[1], dp[0]};
      e2 = {dp[0], dp[2], dp[1]};
    }
    e1 = dp.cross(e1);
    e1 = e1.normalize();
    e2 = dp.cross(e1);
    e2 = e2.normalize();
    for(int i = 0; i < N + 1; i++) {
      const double theta1 = 6.28 * (float)i / N;
      const double theta2 = 6.28 * (float)(i + 1) / N;
      const auto v1 = (e1 * std::cos(theta1) + e2 * std::sin(theta1)) * r;
      const auto v2 = (e1 * std::cos(theta2) + e2 * std::sin(theta2)) * r;
      AddQuad(p - n / 2 + v1, p - n / 2 + v2, p + n / 2 + v2, p + n / 2 + v1, col, width);
    }
  }


  inline void addWidget(::vkUI::uiWidget* w) {
    root_widget.AddWidget(w);
  }
  // [[deprecated]] inline void addWidget(::vkUI::uiWidget w){ root_widget.AddWidget(&w); }
  inline uiWidget& addWidget2D(::vkUI::uiWidget* w) {
    root_widget_ui.AddWidget(w);
    return root_widget_ui;
  }
  // [[deprecated]] inline void addWidget2D(::vkUI::uiWidget w){ root_widget_ui.AddWidget(&w); }
  void AddLine(const Vector3& pos1, const Vector3& pos2, const Vector3b& col1, const Vector3b& col2, const float width = 1.0);
  void AddLine(const Vector3& pos1, const Vector3& pos2, const Vector3b& col, const float width = 1.0);
  void AddPoint(const Vector3& pos, const Vector3b& col, const double size = 1.0);
  void AddPoint_mono_triangle(const Vector3& pos, const Vector3b& col, const double size = 1.0);
  void AddSphere_20(const Vector3& pos, const float size, const Vector3b& col);
  void AddArrow(const Vector3& pos1, const Vector3& pos2, const Vector3b& col, float width = 1.0);
  void AddArrowTo(const Vector3& pos, const Vector3& dir, const Vector3b& col, float width);
  void AddCube(const Vector3& pos, const Vector3& whd, const Vector3b& col);
  void AddCubeLineRotated(const Vector3& pos, const Vector3& size, const Vector3& pry, const Vector3b& col, const float width = 1.0);
  void AddPlane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col);
  void AddPlane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col, int width);
  void AddCubeLine(const Vector3& pos, const Vector3& size, const Vector3b& col, float width = 1.0);
  void AddCubeLine(const Vector3& pos, float size, const Vector3b& col, float width = 1.0);
  void AddCircle(const Vector3& pos, const Vector3& normal, const Vector3b& col);
  void AddCone(const Vector3& pos, const Vector3& dir, float size, const Vector3b& col);
  void AddStringBalloon(const std::string& str, const Vector3& pos, const float size, const Vector3b& col = {255, 255, 255}, const Vector3b& line = {255, 255, 255});
  void AddStringBalloon2D(const std::string& str, const Vector2d& pos, const float size, const Vector3b& col = {255, 255, 255}, const Vector3b& line = {255, 255, 255});
  void AddGridXY(const Vector2 range, const int n = 10, const Vector3b col = {255, 255, 0});
  void AddCross(const Vector3& pos, const int n = 10, const Vector3b col = {255, 255, 0});
  void AddCoord(const Vector3& pos, const Vector3& axis = {0, 0, 1}, const int n = 10);
  Vector2d get_text_size(const std::string& str, float size) const;
#define VKUI_USE_CLIPPING_RECT // TODO: 最終的にはこれなしで動くようにする
  // [ SECTION ] --------  2D Rendering functions ----------
  // VKUI_USE_CLIPPING_RECTがdefineされているときはクリッピングする
  // TODO: stackにしてpop/pushするべきかも
  void __AddPointSizeZero2D(const Vector2d& pos, const Vector3b& col);
  inline void __AddPointSizeZero2D(const Vector2d& pos, const Vector3b& col, const Vector2d& uv) {
    dd.vertices_ui.push_back(std::move(Render::VertexUI(pos, col, uv)));
  }

#ifdef WITH_VULKAN
  inline void AddTriangle2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3, const Vector2d& uv1,
                            const Vector2d& uv2, const Vector2d& uv3) {
    __AddPointSizeZero2D(pos1, col1, uv1);
    __AddPointSizeZero2D(pos3, col3, uv3);
    __AddPointSizeZero2D(pos2, col2, uv2);
  }
  inline void AddTriangle2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3) {
    __AddPointSizeZero2D(pos1, col1);
    __AddPointSizeZero2D(pos3, col3);
    __AddPointSizeZero2D(pos2, col2);
  }
  // TODO: クリッピングする実装
  inline void AddTriangle2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector2d uv1, const Vector2d& uv2, const Vector2d& uv3, const Vector3b& col) {
    __AddPointSizeZero2D(pos1, col, uv1);
    __AddPointSizeZero2D(pos3, col, uv3);
    __AddPointSizeZero2D(pos2, col, uv2);
  }
  inline void AddTriangle2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col) {
    __AddPointSizeZero2D(pos1, col);
    __AddPointSizeZero2D(pos3, col);
    __AddPointSizeZero2D(pos2, col);
  }
#else 
  inline void AddTriangle2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3, const Vector2d& uv1,
                            const Vector2d& uv2, const Vector2d& uv3) {
    __AddPointSizeZero2D(pos1, col1, uv1);
    __AddPointSizeZero2D(pos2, col2, uv2);
    __AddPointSizeZero2D(pos3, col3, uv3);
  }
  inline void AddTriangle2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3) {
    __AddPointSizeZero2D(pos1, col1);
    __AddPointSizeZero2D(pos2, col2);
    __AddPointSizeZero2D(pos3, col3);
  }
  // TODO: クリッピングする実装
  inline void AddTriangle2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector2d uv1, const Vector2d& uv2, const Vector2d& uv3, const Vector3b& col) {
    __AddPointSizeZero2D(pos1, col, uv1);
    __AddPointSizeZero2D(pos2, col, uv2);
    __AddPointSizeZero2D(pos3, col, uv3);
  }
  inline void AddTriangle2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col) {
    __AddPointSizeZero2D(pos1, col);
    __AddPointSizeZero2D(pos2, col);
    __AddPointSizeZero2D(pos3, col);
  }
#endif


  inline void AddQuad2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector2d& pos4, const Vector2d& uv1, const Vector2d& uv2, const Vector2d& uv3, const Vector2d& uv4,
                        const Vector3b& col) {
    AddTriangle2D(pos1, pos2, pos3, uv1, uv2, uv3, col);
    AddTriangle2D(pos1, pos3, pos4, uv1, uv3, uv4, col);
  }
  inline void AddQuad2D(const Vector2d& p1, const Vector2d& p2, const Vector2d& p3, const Vector2d& p4, const Vector3b& col) {
    AddTriangle2D(p1, p2, p3, col);
    AddTriangle2D(p1, p3, p4, col);
  }

  inline void AddRectTB2D(const Vector2d& top, const Vector2d& btm, const Vector3b& col) {
    const Vector2d pos[4] = {
      top,
      Vector2d(top[0], btm[1]),
      btm,
      Vector2d(btm[0], top[1]),
    };
    AddTriangle2D(pos[0], pos[1], pos[2], col);
    AddTriangle2D(pos[0], pos[2], pos[3], col);
  }

  inline void AddRectTB2D(const Vector2d& top, const Vector2d& btm, const Vector3b& col, const float width) {
    const Vector2d pos[4] = {
      top,
      Vector2d(top[0], btm[1]),
      btm,
      Vector2d(btm[0], top[1]),
    };
    AddLine2D(pos[0], pos[1], col, width);
    AddLine2D(pos[1], pos[2], col, width);
    AddLine2D(pos[2], pos[3], col, width);
    AddLine2D(pos[3], pos[0], col, width);
  }
  inline void AddRectPosSize(const Vector2d& pos, const Vector2d& size, const Vector3b& col) {
    AddRectTB2D(pos, pos + size, col);
  }
  inline void AddRectPosSize(const Vector2d& p, const Vector2d& size, const Vector2d& ui1, const Vector2d& ui2, const Vector3b& col = {255, 255, 255}) {
    const auto top = p;
    const auto btm = p + size;

    const Vector2d pos[4] = {
      top,
      Vector2d(top[0], btm[1]),
      btm,
      Vector2d(btm[0], top[1]),
    };

    const Vector2d upos[4] = {
      ui1,
      Vector2d(ui1[0], ui2[1]),
      ui2,
      Vector2d(ui2[0], ui1[1]),
    };

    AddTriangle2D(pos[0], pos[1], pos[2], col, col, col, upos[0], upos[1], upos[2]);
    AddTriangle2D(pos[0], pos[2], pos[3], col, col, col, upos[0], upos[2], upos[3]);
  }

  inline void AddRectPosSize(const Vector2d& p, const Vector2d& size, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3, const Vector3b& col4) {
    const auto top = p;
    const auto btm = p + size;
    const Vector2d pos[4] = {
      top,
      Vector2d(top[0], btm[1]),
      btm,
      Vector2d(btm[0], top[1]),
    };
    AddTriangle2D(pos[0], pos[1], pos[2], col1, col2, col3);
    AddTriangle2D(pos[0], pos[2], pos[3], col1, col3, col4);
  }

  void AddRotatedRectPosSize(const Vector2d& pos, const Vector2d& size, const double theta, const Vector3b& col);
  void AddRotatedRectPosSize(const Vector2d& pos, const Vector2d& size, const double theta, const Vector3b& col, const float width = 2.0f);
  inline void AddRectPosSize(const Vector2d& pos, const Vector2d& size, const Vector3b& col, float width) {
    AddRectTB2D(pos, pos + size, col, width);
  }
  inline void AddLine2D(const Vector2d& pos1, const Vector2d& pos2, const Vector3b& col1, const Vector3b& col2, const float width = 1.0) {
    // const auto dpd = pos2 - pos1;
    // const auto dp = Vector2{ (double)dpd[0], (double)dpd[1] } / dpd.norm();
    const auto dp = pos2 - pos1;
    const Vector2d e = {(int)(dp[1] * width / dp.norm()), -(int)(dp[0] * width / dp.norm())};
    AddTriangle2D(pos1, pos2, pos2 + e, col1, col2, col2);
    AddTriangle2D(pos1, pos2 + e, pos2, col1, col2, col2);
    AddTriangle2D(pos1, pos2 + e, pos1 + e, col1, col2, col1);
    AddTriangle2D(pos1, pos1 + e, pos2 + e, col1, col2, col1);
  }
  inline void AddLine2D(const Vector2d& pos1, const Vector2d& pos2, const Vector3b& col, const float width = 1.0) {
    AddLine2D(pos1, pos2, col, col, width);
  }
  void AddArrow2D(const Vector2d& from, const Vector2d& to, const Vector3b& col, const float width = 1.0);

  inline void AddQuad2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector2d& pos4, const Vector3b& col, const float width) {
    AddLine2D(pos1, pos2, col, width);
    AddLine2D(pos2, pos3, col, width);
    AddLine2D(pos3, pos4, col, width);
    AddLine2D(pos4, pos1, col, width);
  }

  void AddCheck2D(const Vector2d& pos, const int size, const Vector3b& col, float line_width) {
    AddLine2D(pos + Vector2({(float)size * 0.1, (float)size * 0.5}), pos + Vector2{(float)size * 0.5, (float)size * 0.9}, col, line_width);
    AddLine2D(pos + Vector2({(float)size * 0.5, (float)size * 0.9}), pos + Vector2{(float)size * 0.9, (float)size * 0.1}, col, line_width);
  }

  void AddArrowDown2D(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    AddTriangle2D(pos, pos + Vector2d{size / 2, h}, pos + Vector2d{size, 0}, col);
  }

  void AddArrowUp2D(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    AddTriangle2D(pos + Vector2d{0, h}, pos + Vector2d{size, h}, pos + Vector2d{size / 2, 0}, col);
  }

  void AddArrowLeft2D(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    AddTriangle2D(pos + Vector2d{0, size / 2}, pos + Vector2d{h, size}, pos + Vector2d{h, 0}, col);
  }

  void AddArrowRight2D(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    AddTriangle2D(pos + Vector2d{0, size}, pos + Vector2d{h, size}, pos + Vector2d{h, 0}, col);
  }
  void AddArrowRight2D2(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    AddTriangle2D(pos, pos + Vector2d{0, size}, pos + Vector2d{h, size / 2}, col);
  }

  void AddCrossButton(const Vector2d& pos, const int size, const Vector3b& bg_col, const Vector3b& line_col, const Vector3b& cross_col) {
    AddRectTB2D(pos, pos + size, bg_col);
    AddRectTB2D(pos, pos + size, line_col, 1);
    constexpr int _padding_cross = 2;
    constexpr int _cross_line_width = 2;
    AddLine2D(pos + _padding_cross, pos + size - _padding_cross, {255, 255, 255}, _cross_line_width);
    AddLine2D(pos + Vector2d{size - _padding_cross, _padding_cross}, pos + Vector2d{_padding_cross, size - _padding_cross}, cross_col, _cross_line_width);
  }

  inline void AddCross2D(const Vector2d& center, const int size, const Vector3b& col) {
    constexpr int _cross_line_width = 2;
    const Vector2d pos = center - int(size / 2);
    AddLine2D(pos, pos + size, col, _cross_line_width);
    AddLine2D(pos + Vector2d{size, 0}, pos + Vector2d{0, size}, col, _cross_line_width);
  }

  inline void AddPlus2D(const Vector2d& center, const int size, const Vector3b& col) {
    constexpr int _cross_line_width = 2;
    const int hs = size / 2; // half size
    AddLine2D(center - Vector2d(hs, 0), center + Vector2d(hs, 0), col, _cross_line_width);
    AddLine2D(center - Vector2d(0, hs), center + Vector2d(0, hs), col, _cross_line_width);
  }

  inline void AddDiamond2D(const Vector2d& center, const int size, const Vector3b& col) {
    const int hs = size / 2;
    AddQuad2D(center + Vector2d{hs, 0}, center + Vector2d{0, hs}, center - Vector2d{hs, 0}, center - Vector2d{0, hs}, col);
  }

  void AddCircle2D(const Vector2d& pos, const int size, const Vector3b col) {
    if(size < 15) {
      constexpr double s45 = 0.707090402;
      const Vector2 p[8] = {{0, 1}, {s45, s45}, {1, 0}, {s45, -s45}, {0, -1}, {-s45, -s45}, {-1, 0}, {-s45, s45}};
      for(int i = 0; i < 6; i++) AddTriangle2D(pos + p[0] * size, pos + p[1 + i] * size, pos + p[i + 2] * size, col);
    } else {
      constexpr double dth = 6.28 / 20.0f;
      for(int i = 0; i < 20; i++) {
        const auto p1 = pos + Vector2d(std::cos(dth * i) * size, std::sin(dth * i) * size);
        const auto p2 = pos + Vector2d(std::cos(dth * (i + 1)) * size, std::sin(dth * (i + 1)) * size);
        AddTriangle2D(pos, p1, p2, col);
      }
    }
  }

  void AddCircle2D(const Vector2d& pos, const int size, const Vector3b col, const int width) {
    constexpr double dth = 6.28 / 20.0f;
    for(int i = 0; i < 20; i++) {
      const auto p1 = pos + Vector2d(std::cos(dth * i) * size, std::sin(dth * i) * size);
      const auto p2 = pos + Vector2d(std::cos(dth * (i + 1)) * size, std::sin(dth * (i + 1)) * size);
      AddLine2D(p1, p2, col, width);
    }
  }
#ifdef VKUI_USE_CLIPPING_RECT
  inline void AddQuad2D_clip(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector2d& pos4, const Vector2d& uv1, const Vector2d& uv2, const Vector2d& uv3, const Vector2d& uv4,
                             const Vector3b& col) {
    const Vector2d pos1_{
      std::clamp<int>(pos1[0], clipping_rect.posX, clipping_rect.right()),
      std::clamp<int>(pos1[1], clipping_rect.posY, clipping_rect.bottom()),
    };
    const Vector2d pos2_{
      std::clamp<int>(pos2[0], clipping_rect.posX, clipping_rect.right()),
      std::clamp<int>(pos2[1], clipping_rect.posY, clipping_rect.bottom()),
    };
    const Vector2d pos3_{
      std::clamp<int>(pos3[0], clipping_rect.posX, clipping_rect.right()),
      std::clamp<int>(pos3[1], clipping_rect.posY, clipping_rect.bottom()),
    };
    const Vector2d pos4_{
      std::clamp<int>(pos4[0], clipping_rect.posX, clipping_rect.right()),
      std::clamp<int>(pos4[1], clipping_rect.posY, clipping_rect.bottom()),
    };
    AddTriangle2D(pos1_, pos2_, pos3_, uv1, uv2, uv3, col);
    AddTriangle2D(pos1_, pos3_, pos4_, uv1, uv3, uv4, col);
  }

  inline void AddRectTB2D_clip(const Vector2d& top, const Vector2d& btm, const Vector3b& col) {
    const Vector2d top_clipped{std::max<int>(top[0], clipping_rect.posX), std::max<int>(top[1], clipping_rect.posY)};
    const Vector2d btm_clipped{std::min<int>(btm[0], clipping_rect.right()), std::min<int>(btm[1], clipping_rect.bottom())};
    const Vector2d pos[4] = {
      top_clipped,
      Vector2d(top_clipped[0], btm_clipped[1]),
      btm_clipped,
      Vector2d(btm_clipped[0], top_clipped[1]),
    };
    AddTriangle2D(pos[0], pos[1], pos[2], col);
    AddTriangle2D(pos[0], pos[2], pos[3], col);
  }

  inline void AddRectTB2D_clip(const Vector2d& top, const Vector2d& btm, const Vector3b& col, const float width) {
    const Vector2d top_clipped{std::max<int>(top[0], clipping_rect.posX), std::max<int>(top[1], clipping_rect.posY)};
    const Vector2d btm_clipped{std::min<int>(btm[0], clipping_rect.right()), std::min<int>(btm[1], clipping_rect.bottom())};
    const Vector2d pos[4] = {
      top_clipped,
      Vector2d(top_clipped[0], btm_clipped[1]),
      btm_clipped,
      Vector2d(btm_clipped[0], top_clipped[1]),
    };
    AddLine2D(pos[0], pos[1], col, width);
    AddLine2D(pos[1], pos[2], col, width);
    AddLine2D(pos[2], pos[3], col, width);
    AddLine2D(pos[3], pos[0], col, width);
  }

  inline void AddRectPosSize_clip(const Vector2d& pos, const Vector2d& size, const Vector3b& col) {
    AddRectTB2D_clip(pos, pos + size, col);
  }
  inline void AddRectPosSize_clip(const Vector2d& pos, const Vector2d& size, const Vector3b& col, float width) {
    AddRectTB2D_clip(pos, pos + size, col, width);
  }
#endif

  Vector2d AddString2D(const std::string& str, const Vector2d& pos, const float size, const Vector3b& col = {255, 255, 255}, const int xlim = std::numeric_limits<int>::max());
  Vector2d AddString2D(const std::string& str, const Vector2d& pos, const float size, const uiRect& clip_rect, const Vector3b& col = {255, 255, 255});

  inline void setCameraPos(const Vector3 pos) {
    camera_position.pos = pos;
  }
  inline void setCameraTarget(const Vector3 target) {
    camera_position.dir = target;
  }
  inline auto getCameraPos() const {
    return camera_position.pos;
  }
  inline auto getCameraTarget() const {
    return camera_position.dir;
  }
  inline void setCameraUpVector(const Vector3 up) {
    camera_position.u = up;
  }
  inline void setCameraScale(const float scale) {
    camera_position.scale = scale;
  }
  inline void setCameraZclip(const double near, const double far) {
    camera_position.zNear = near;
    camera_position.zFar = far;
  }
  inline void setCameraAspect(const double aspect) {
    camera_position.aspect = aspect;
  }
  void terminate();
};


// -----------------------------------------------------
//    [SECTION] uiFont
// -----------------------------------------------------


// 1つの文字（グリフ）に対するデータ
struct uiGlyph {
  // unsigned int    Codepoint : 31;     // 0x0000..0xFFFF
  // unsigned int    Visible : 1;        // Flag to allow early out when rendering
  unsigned int U0, V0, U1, V1; // UVテクスチャの座標
  int dHeight;                 // グリフの最上点から基準ラインまでの高さ
};


enum class FontLanguage {
  Japansese,
  English,
  Chinese,
  Korean,
  Thai,
  Vietnamese,
  // JPN,ENG,CHI,
};


using uiWchar = unsigned short;

class uiFont {
  // private:
public:
  FT_Library library;
  FT_Face face;
  FT_GlyphSlot slot; // グリフへのショートカット

  // uiFontFlags    flags;
  unsigned short desiredTextSize;

  float Spacing;                   // 字間、config()関数内でuiStyle->FontSpacingの値が設定される
  float FontSize;                  // フォントサイズ、config()関数内でuiStyle->FontSpacingの値が設定される　
  unsigned int TexWidth;           // build()関数で作成される文字テクスチャのサイズ＜GL_MAX_TEXTURE_SIZE.
  unsigned int TexHeight;          // build()関数で作成される文字テクスチャのサイズ＜GL_MAX_TEXTURE_SIZE.
  unsigned int TexHeight_capacity; // 予約済み　のテクスちゃ老域の全体高さ
  bool isBuildFinished;            // フォントの作成が終わったかどうか
  FontLanguage language;
  uiWchar *GlyphRanges, *IconGlyphRanges;
  unsigned int nGlyph, nIconGlyph; // GlyphRangesの配列の数（つまりグリフの数）
  std::string FontName;
  std::string iconFontName;

  // bool              MouseCursorIcons;   //
  Vector2d TexUvWhitePixel;      // Texture coordinates to a white pixel
  uiVector<uiWchar> IndexLookup; // 12-16 // out //            // Sparse. Index glyphs by Unicode code-point.
  uiVector<uiGlyph> Glyphs;      // すべての文字に対するグリフ
  uiGlyph* FallbackGlyph;        // FinGlyphで上手くいかなかったときのGlyph（□の文字化けのやつ）

  unsigned char* _Data; // テクスチャデータ
                        // float Scale;
  void AddGlyph(uiWchar c, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x);
  void AddRemapChar(uiWchar dst, uiWchar src, bool overwrite_dst = true); // Makes 'dst' character/glyph points to 'src' character/glyph. Currently needs to be called AFTER fonts have been built.
  void SetGlyphVisible(uiWchar c, bool visible);
  bool IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);

  uiWchar* GetGlyphRangesKorean();
  uiWchar* GetGlyphRangesJapanese();
  uiWchar* GetGlyphRangesEnglish();
  uiWchar* GetGlyphRangesChinese();
  uiWchar* GetGlyphRangesCyrillic();
  uiWchar* GetGlyphRangesThai();
  uiWchar* GetGlyphRangesVietnamese();
  uiWchar* getGlyphRangeIcon();

public:
  uiFont();
  ~uiFont();
  void init();
  bool setLanguage(FontLanguage l);
  void setStyle(uiStyle* style);
  bool build();                  //フォンﾄトをレンダリングする
  uiGlyph* FindGlyph(uiWchar c); //
  bool getSize(uiWchar c, Vector2d* size, unsigned int* dH);
  Vector2f CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end = NULL, const char** remaining = NULL) const; // utf8
  char* CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const;

  inline unsigned int getTexWidth() { return TexWidth; }
  inline unsigned int getTexHeight() { return TexHeight; }
  inline unsigned char* getData() { return _Data; }
  inline unsigned int getGlyphNum() { return Glyphs.size(); }
  void setFontSize(const int s);
  void setFontSpacing(const int s);
  void setFontName(const std::string& s);
  void setIconFontName(const std::string& s);

  bool build_internal(const std::string& fontname, const int fontsize, const uiWchar* glyphRange, const int n);
};



// -----------------------------------------------------
//    [SECTION] Engine
// -----------------------------------------------------
struct uiEngine {
  std::vector<uiWindow*> windows;
  uiRenderer renderer;

  uiWindow *drawingWnd, *hoveringWnd, *focusedWnd;
  uiFont text_renderer;
  uiStyle style;

  void init() {
    if(!glfwInit()) {
      std::cerr << "ERROR: could not start GLFW3\n";
      return;
    }
#ifdef WITH_OPENGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
    text_renderer.init();
    text_renderer.setLanguage(vkUI::FontLanguage::Japansese);
    text_renderer.build();
  }

  bool update(const bool verbose) {
    assert(windows.size() > 0);
    glfwPollEvents();
    for(int i = 0; i < windows.size(); i++) {
      windows[i]->drawFrame(verbose);
    }
#if 0
    (*renderer.get_device_ptr())->waitIdle();
#endif
    return !windows[0]->wndShouldClose();
  }

  void cleanup() {
    renderer.terminate();
    // glfwDestroyWindow(window);  // TODO:
    glfwTerminate();
  }

  void terminate(const bool verbose = false) {
    if(verbose) {
      std::cout << "terminating vkui......." << std::endl;
    }
    cleanup();
    if(verbose) {
      std::cout << "deleting windows ........" << std::endl;
    }
    for(int i = 0; i < windows.size(); i++) {
      windows[i]->terminate();
    }
    if(verbose) {
      std::cout << "deleting windows ........" << std::endl;
    }
    return;
  }
  void updateVertexBuffer();
};

#define VKUI_ENGINE_API

extern uiEngine engine;
inline VKUI_ENGINE_API uiEngine* getContextPtr() {
  return &engine;
}

inline VKUI_ENGINE_API auto setDrawingWindow(uiWindow* wnd) {
  return engine.drawingWnd = wnd;
}
inline VKUI_ENGINE_API auto getDrawingWindow() {
  return engine.drawingWnd;
}
inline VKUI_ENGINE_API auto setFocusedWindow(uiWindow* wnd) {
  return engine.focusedWnd = wnd;
}
inline VKUI_ENGINE_API auto getFocusedWindow() {
  return engine.focusedWnd;
}
inline VKUI_ENGINE_API auto setHoveringWindow(uiWindow* wnd) {
  return engine.hoveringWnd = wnd;
}
inline VKUI_ENGINE_API auto getHoveringWindow() {
  return engine.hoveringWnd;
}

// inline VKUI_ENGINE_API auto setFocusedWidget(uiWidget *w){return engine.focusedWidget = w;}
inline VKUI_ENGINE_API uiWidget* getFocusedWidget() {
  return getDrawingWindow()->root_widget_ui.getFocusedWidget();
}
inline VKUI_ENGINE_API uiWidget* getHoveringWidget() {
  return getDrawingWindow()->root_widget_ui.getHoveringWidget();
}

inline VKUI_ENGINE_API auto getTextRendererPtr() {
  return &(engine.text_renderer);
};

inline VKUI_ENGINE_API auto getWhitePixel(){ return engine.text_renderer.TexUvWhitePixel; }

// initialize uiContext and create default window
inline VKUI_ENGINE_API void init() {
  engine.init();
}
inline VKUI_ENGINE_API void initFinish() {
  assert(engine.windows.size() > 0);
  engine.renderer.init();
}
inline VKUI_ENGINE_API uiWindow* addWindow(std::string name, int w, int h) {
  const auto A = new uiWindow(name, w, h);
  engine.windows.push_back(A);
  return A;
}
inline VKUI_ENGINE_API bool render(const bool verbose = false) {
  return engine.update(verbose);
}
inline VKUI_ENGINE_API void Terminate(const bool verbose = false) {
  engine.terminate(verbose);
}

// styles
inline VKUI_ENGINE_API const uiStyle* getStyle() {
  return &engine.style;
}
inline VKUI_ENGINE_API void setStyle(const uiStyle& s) {
  engine.style = s;
}

inline VKUI_ENGINE_API auto getAllWindows() {
  return &engine.windows;
}

} // namespace vkUI::Engine
