#include <engine.hpp>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <type_traits>
#include <widget.hpp>
#ifdef DEFINED_STD_FORMAT
#include <format>
#else
#include <functional>
#include <iostream>
#include <logger.hpp>
#include <string>
#include <utility>

template <typename... Args> std::string myFormat(const std::string& base_str, const Args... args) {
  std::string base_str_ = base_str;
  /* constexpr std::size_t size = sizeof...(Args); */
  auto tt = std::make_tuple(args...);
  std::apply(
    [&](auto&&... args_) {
      ((base_str_.replace(base_str_.find("{}") != std::string::npos ? base_str_.find("{}") : 0, base_str_.find("{}") == std::string::npos ? 0 : 2, std::to_string(args_))), ...);
    },
    tt);
  return base_str_;
}
#endif
#include <chrono>

namespace vkUI::Engine {
uiWindow::uiWindow(std::string _name, uint16_t width, uint16_t height) {
  window = glfwCreateWindow(width, height, _name.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, resizeCB_static);
  glfwSetCursorPosCallback(window, mouseCB_static);
  glfwSetKeyCallback(window, keyboardCB_static);
  glfwSetScrollCallback(window, scrollCB_static);
  glfwSetCharCallback(window, charCB_static);
  glfwSetMouseButtonCallback(window, mouseButtonCB_static);
  currentFrame = 0;
  user_key_cb = [](int a, int b, int c, int d, Vector2 f) { return true; };
  uiLOGI << "updateUniformBuffer";
  updateUniformBuffer();
  uiLOGI << "vertexbuffer";
  updateVertexBuffer();
  uiLOGI << "cursor init";
  cursors.init();
  root_widget_ui.needRendering(true);
  root_widget_ui.impl_needCalcAlignment_child();
  root_widget_ui.impl_needCalcInnerSize_parent();
  uiLOGI << "constructor end";
}


void uiWindow::updateVertexBuffer() {
  setDrawingWindow(this);
  /* renderer.updateVertexBuffer(); */
  root_widget_ui.calcInnerSize_recursize();
  root_widget_ui.applyAlignment_recursive();
  // if(root_widget_ui.getNeedRendering()){
  root_widget_ui.needRendering(true);

  dd.clear();
  root_widget.render();
  root_widget_ui.render_child_widget();

#if 0
  if(dd.get_n_vertices_ui() == 0) {
    dd.add({0, 0}, {0, 0, 0}, {0, 0});
    dd.add({0, 0}, {0, 0, 0}, {0, 0});
    dd.add({0, 0}, {0, 0, 0}, {0, 0});
  }
#endif
}

void uiWindow::updateUniformBuffer() {
  int ww, wh;
  glfwGetWindowSize(window, &ww, &wh);

  camera_position.getCameraProjMatrix(dd.uniform_data.proj);
#if 0
    const float projectionMatrix[] = { // 左下原点
        2.0f / float(ww), 0.0f, 0.0f, 0.0f ,
        0.0f, -2.0f / float(wh), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
#else
  const float projectionMatrix[] = {// 左上原点
                                    2.0f / float(ww), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / float(wh), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f};
#endif
  for(int i = 0; i < 16; i++) {
    dd.uniform_data.proj_uv[i] = projectionMatrix[i];
  }
  const auto text_renderer = getTextRendererPtr();
  dd.uniform_data.texure_size[0] = text_renderer->TexWidth;
  dd.uniform_data.texure_size[1] = text_renderer->TexHeight;

  // std::cout << std::fixed;
  //     for(int i=0; i<4; i++){
  //         for(int j=0; j<4; j++){std::cout << std::setprecision(3) << uniform_data.proj_uv[i*4+j] << ", "; }
  //         std::cout << std::endl;
  //     }
  // std::cout << "position = " << camera_position.pos << std::endl;
  // std::cout << "target = " << camera_position.dir << std::endl;
}

void uiWindow::drawDevelopperHelps() {
  const auto nCmd = dd.drawlist.size();
  const auto wsize = root_widget_ui.get_widget_num();
#ifdef DEFINED_STD_FORMAT
  const std::string str = std::format(
#else
  const std::string str = myFormat(
#endif
    "vert_size = (3d = {}, ui = {}), drawCmd={} nWidget={}, fps={}", dd.get_n_vertices_ui(), dd.get_n_vertices(), nCmd, wsize, fps);
  AddString2D(str, {10, 10}, 1, {255, 0, 255});

  const auto focused = root_widget_ui.getFocusedWidget();
  const auto hovering = root_widget_ui.getHoveringWidget();
  AddRectPosSize(hovering->getPos(), hovering->getSize(), {0, 255, 0}, 2);
  AddRectPosSize(focused->getPos(), focused->getSize(), {255, 0, 0}, 1);
}

void uiWindow::drawFrame(const bool verbose) {
  setDrawingWindow(this);
  updateUniformBuffer();
  dd.push();
  dd.sort();
  renderer.draw(&dd);
  dd.clear();
#ifdef VKUI_ENGINE_ENABLE_FPS_CALC
  // static int frames_in_sec = 0;
  static std::chrono::system_clock::time_point start_time_fps = std::chrono::system_clock::now();
  const auto now = std::chrono::system_clock::now();
  const double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(start_time_fps - now).count();
  fps = (float)1e6 / elapsed;
  start_time_fps = now;
  // frames_in_sec++;
#endif
  glfwSwapBuffers(window);

  currentFrame++;
}

void uiWindow::init() {
  std::cout << "uiWIndow init" << std::endl;
  renderer.init();
  std::cout << "uiWIndow init end" << std::endl;
}

void uiWindow::mouseCB(double x, double y) {
  static bool wnd_rotating = false;
  // static Vector3 start_pos_mouse, start_pos_camera;
  static double radius;
  static Vector2 rotation, start_pos_mouse;
  int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
  if(state == GLFW_PRESS) {
    const auto vecc = camera_position.pos - camera_position.dir;
    if(!wnd_rotating) {
      radius = vecc.norm();
      rotation[0] = std::acos(vecc[2] / radius);  // theta
      rotation[1] = std::atan2(vecc[1], vecc[0]); // phi
      start_pos_mouse = {x, y};
      // start_pos_camera = camera_position.pos;
    }
    const auto delta = Vector2(x, y) - start_pos_mouse;
    const double theta = rotation[0] + delta[0] / 250;
    const double phi = rotation[1] + delta[1] / 250;
    const Vector3 tmp = {radius * std::sin(theta) * std::cos(phi), radius * std::sin(theta) * std::sin(phi), radius * std::cos(theta)};
    camera_position.pos = tmp + camera_position.dir;
    wnd_rotating = true;
  } else if(wnd_rotating) {
    wnd_rotating = false;
  }
  int state_ = 0;
  state_ |= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
  state_ |= (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) << 1;
  state_ |= (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) << 2;
  root_widget_ui.CallbackFunc(uiCallbackFlags::MouseMove, {(int)x, (int)y}, 0, state_, nullptr);
}

void uiWindow::scrollCB(double xoffset, double yoffset) {
  std::cout << "scroll " << xoffset << ", " << yoffset << std::endl;
  camera_position.go_closer(yoffset);
  root_widget_ui.CallbackFunc(uiCallbackFlags::MouseScroll, {0, 0}, xoffset, yoffset, nullptr);
}


// ----------------------------------------------------------------
//     [ SECTION ] Drawing Functions
// ----------------------------------------------------------------

void uiWindow::AddLine(const Vector3& pos1, const Vector3& pos2, const Vector3b& col1, const Vector3b& col2, const float width) {
  auto dp = pos2 - pos1;
  dp = dp / dp.norm();
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
  e1 *= width;
  e2 *= width;
  AddTriangle(pos1, pos1 + e1 + e2, pos2 + e1, col1, col1, col2);
  AddTriangle(pos1 + e1 + e2, pos2 + e2, pos2 + e1, col1, col2, col2); // 2-4-3
  AddTriangle(pos1 + e1 + e2, pos1, pos2 + e2, col1, col1, col2);      // 214
  AddTriangle(pos1, pos2 + e1, pos2 + e2, col1, col2, col2);           // 134
}

void uiWindow::AddLine(const Vector3& pos1, const Vector3& pos2, const Vector3b& col, const float width) {
  AddLine(pos1, pos2, col, col, width);
}

void uiWindow::AddPoint(const Vector3& pos, const Vector3b& col, const double size) {
  const Vector3 points[] = {
    {pos + Vector3({0.0f, 0.0f, size})},
    {pos + Vector3({size, 0.0f, 0.0f})},
    {pos + Vector3({0.0f, size, 0.0f})},
    {pos + Vector3({size, size, size})},
  };
  AddTriangle(points[0], points[1], points[2], col);
  AddTriangle(points[2], points[1], points[3], col);
  AddTriangle(points[0], points[3], points[1], col);
  AddTriangle(points[2], points[3], points[0], col);
}

void uiWindow::AddPoint_mono_triangle(const Vector3& pos, const Vector3b& col, const double size) {
  AddTriangle(pos + Vector3({size, 0.0f, 0.0f}), pos + Vector3({0.0f, 0.0f, size}), pos + Vector3({0.0f, size, 0.0f}), col);
  AddTriangle(pos + Vector3({0.0f, 0.0f, size}), pos + Vector3({size, 0.0f, 0.0f}), pos + Vector3({0.0f, size, 0.0f}), col);
}

void uiWindow::AddSphere_20(const Vector3& pos, const float size, const Vector3b& col) {
  // 正20面体
  const double t = (1 + sqrt(5)) / 2;
  const Vector3 points[] = {
    pos + Vector3(0.0f, -1.0f, t) * size,  pos + Vector3(0.0f, 1.0f, t) * size,

    pos + Vector3(t, 0.0f, 1.0f) * size,   pos + Vector3(-t, 0.0f, 1.0f) * size,

    pos + Vector3(1.0f, -t, 0.0f) * size,  pos + Vector3(1.0f, t, 0.0f) * size,   pos + Vector3(-1.0f, t, 0.0f) * size, pos + Vector3(-1.0f, -t, 0.0f) * size,

    pos + Vector3(t, 0.0f, -1.0f) * size,  pos + Vector3(-t, 0.0f, -1.0f) * size,

    pos + Vector3(0.0f, -1.0f, -t) * size, pos + Vector3(0.0f, 1.0f, -t) * size,
  };
  float width = -1;
  AddTriangle(points[0], points[2], points[1], col, width);
  AddTriangle(points[1], points[2], points[5], col, width);
  AddTriangle(points[1], points[5], points[6], col, width);
  AddTriangle(points[1], points[6], points[3], col, width);
  AddTriangle(points[1], points[3], points[0], col, width);
  AddTriangle(points[0], points[3], points[7], col, width);
  AddTriangle(points[0], points[7], points[4], col, width);
  AddTriangle(points[0], points[4], points[2], col, width);

  AddTriangle(points[11], points[8], points[10], col, width);
  AddTriangle(points[11], points[5], points[8], col, width);
  AddTriangle(points[11], points[6], points[5], col, width);
  AddTriangle(points[11], points[9], points[6], col, width);
  AddTriangle(points[11], points[10], points[9], col, width);
  AddTriangle(points[9], points[10], points[7], col, width);
  AddTriangle(points[10], points[4], points[7], col, width);
  AddTriangle(points[10], points[8], points[4], col, width);

  AddTriangle(points[5], points[2], points[8], col, width);
  AddTriangle(points[3], points[6], points[9], col, width);
  AddTriangle(points[7], points[3], points[9], col, width);
  AddTriangle(points[2], points[4], points[8], col, width);
}

void uiWindow::AddArrow(const Vector3& pos1, const Vector3& pos2, const Vector3b& col, float width) {
  AddLine(pos1, pos2, col, width);
  const double len = (pos1 - pos2).norm();
  AddSphere_20(pos2, len / 80, col);
}

void uiWindow::AddArrowTo(const Vector3& pos, const Vector3& dir, const Vector3b& col, float width) {
  AddLine(pos, pos + dir, col, width);
  const double len = dir.norm();
  AddSphere_20(pos + dir, len / 80, col);
}

void uiWindow::AddCube(const Vector3& pos, const Vector3& whd, const Vector3b& col) {
  const auto half_size = whd / 2;
  const Vector3 points[8] = {
    pos + Vector3(-half_size[0], -half_size[1], -half_size[2]), pos + Vector3(-half_size[0], -half_size[1], half_size[2]), pos + Vector3(-half_size[0], half_size[1], half_size[2]),
    pos + Vector3(-half_size[0], half_size[1], -half_size[2]),  pos + Vector3(half_size[0], -half_size[1], -half_size[2]), pos + Vector3(half_size[0], -half_size[1], half_size[2]),
    pos + Vector3(half_size[0], half_size[1], half_size[2]),    pos + Vector3(half_size[0], half_size[1], -half_size[2]),
  };
  AddQuad(points[0], points[1], points[2], points[3], col);
  AddQuad(points[3], points[2], points[6], points[7], col);
  AddQuad(points[7], points[6], points[5], points[4], col);
  AddQuad(points[4], points[5], points[1], points[0], col);
  AddQuad(points[0], points[3], points[7], points[4], col);
  AddQuad(points[5], points[6], points[2], points[1], col);
}

void uiWindow::AddCubeLineRotated(const Vector3& pos, const Vector3& size, const Vector3& pry, const Vector3b& col, const float width) {
  const auto half_size = size / 2;
  const Mat3x3 rot0{1, 0, 0, 0, std::cos(pry[0]), -std::sin(pry[0]), 0, std::sin(pry[0]), std::cos(pry[0])};
  const Mat3x3 rot1{std::cos(pry[1]), 0, std::sin(pry[1]), 0, 1, 0, -std::sin(pry[1]), 0, std::cos(pry[1])};
  const Mat3x3 rot2{
    std::cos(pry[2]), -std::sin(pry[2]), 0, std::sin(pry[2]), std::cos(pry[2]), 0, 0, 0, 1,
  };
  const auto rot = rot0 * rot1 * rot2;
  const Vector3 points[8] = {
    pos + rot * Vector3(-half_size[0], -half_size[1], -half_size[2]), pos + rot * Vector3(-half_size[0], -half_size[1], half_size[2]), pos + rot * Vector3(-half_size[0], half_size[1], half_size[2]),
    pos + rot * Vector3(-half_size[0], half_size[1], -half_size[2]),  pos + rot * Vector3(half_size[0], -half_size[1], -half_size[2]), pos + rot * Vector3(half_size[0], -half_size[1], half_size[2]),
    pos + rot * Vector3(half_size[0], half_size[1], half_size[2]),    pos + rot * Vector3(half_size[0], half_size[1], -half_size[2]),
  };
  AddLine(points[0], points[1], col, width);
  AddLine(points[1], points[2], col, width);
  AddLine(points[2], points[3], col, width);
  AddLine(points[3], points[0], col, width);
  AddLine(points[4], points[5], col, width);
  AddLine(points[5], points[6], col, width);
  AddLine(points[6], points[7], col, width);
  AddLine(points[0], points[4], col, width);
  AddLine(points[1], points[5], col, width);
  AddLine(points[2], points[6], col, width);
  AddLine(points[3], points[7], col, width);
}

void uiWindow::AddCubeLine(const Vector3& pos, const Vector3& size, const Vector3b& col, float width) {
  const auto half_size = size / 2;
  const Vector3 points[8] = {
    pos + Vector3(-half_size[0], -half_size[1], -half_size[2]), pos + Vector3(-half_size[0], -half_size[1], half_size[2]), pos + Vector3(-half_size[0], half_size[1], half_size[2]),
    pos + Vector3(-half_size[0], half_size[1], -half_size[2]),  pos + Vector3(half_size[0], -half_size[1], -half_size[2]), pos + Vector3(half_size[0], -half_size[1], half_size[2]),
    pos + Vector3(half_size[0], half_size[1], half_size[2]),    pos + Vector3(half_size[0], half_size[1], -half_size[2]),
  };
  AddLine(points[0], points[1], col, width);
  AddLine(points[1], points[2], col, width);
  AddLine(points[2], points[3], col, width);
  AddLine(points[3], points[0], col, width);
  AddLine(points[4], points[5], col, width);
  AddLine(points[5], points[6], col, width);
  AddLine(points[6], points[7], col, width);
  AddLine(points[0], points[4], col, width);
  AddLine(points[1], points[5], col, width);
  AddLine(points[2], points[6], col, width);
  AddLine(points[3], points[7], col, width);
}
void uiWindow::AddCubeLine(const Vector3& pos, float size, const Vector3b& col, float width) {
  AddCubeLine(pos, Vector3(size, size, size), col, width);
}

void uiWindow::AddPlane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col) {
  Vector3 e1, e2;
  const auto dp = normal.normalize();
  if(normal[0] >= normal[1] && normal[0] >= normal[2]) { // X軸に近い直線
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[2], normal[1], normal[0]};
  } else if(normal[1] >= normal[0] && normal[1] >= normal[2]) {
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[0], normal[2], normal[1]};
  } else {
    e1 = {normal[2], normal[1], normal[0]};
    e2 = {normal[0], normal[2], normal[1]};
  }
  e1 = dp.cross(e1);
  e1 = e1.normalize() / 2.0f;
  e2 = dp.cross(e1);
  e2 = e2.normalize() / 2.0f;
  const Vector3 p[4] = {
    pos + (-e1 - e2) * size[0],
    pos + (e1 - e2) * size[1],
    pos + (e1 + e2) * size[1],
    pos + (-e1 + e2) * size[1],
  };
  AddQuad(p[0], p[1], p[2], p[3], col);
  AddQuad(p[3], p[2], p[1], p[0], col);
}

void uiWindow::AddPlane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col, int width) {
  Vector3 e1, e2;
  const auto dp = normal.normalize();
  if(normal[0] >= normal[1] && normal[0] >= normal[2]) { // X軸に近い直線
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[2], normal[1], normal[0]};
  } else if(normal[1] >= normal[0] && normal[1] >= normal[2]) {
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[0], normal[2], normal[1]};
  } else {
    e1 = {normal[2], normal[1], normal[0]};
    e2 = {normal[0], normal[2], normal[1]};
  }
  e1 = dp.cross(e1);
  e1 = e1.normalize() / 2.0f;
  e2 = dp.cross(e1);
  e2 = e2.normalize() / 2.0f;
  const Vector3 p[4] = {
    pos + (-e1 - e2) * size[0],
    pos + (e1 - e2) * size[1],
    pos + (e1 + e2) * size[1],
    pos + (-e1 + e2) * size[1],
  };
  AddLine(p[0], p[1], col, width);
  AddLine(p[1], p[2], col, width);
  AddLine(p[2], p[3], col, width);
  AddLine(p[3], p[0], col, width);
}

void uiWindow::AddCircle(const Vector3& pos, const Vector3& normal, const Vector3b& col) {
  const auto dp = normal.normalize();
  constexpr int N = 25; // 分割数
  Vector3 e1, e2;
  if(normal[0] >= normal[1] && normal[0] >= normal[2]) { // X軸に近い直線
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[2], normal[1], normal[0]};
  } else if(normal[1] >= normal[0] && normal[1] >= normal[2]) {
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[0], normal[2], normal[1]};
  } else {
    e1 = {normal[2], normal[1], normal[0]};
    e2 = {normal[0], normal[2], normal[1]};
  }

  e1 = dp.cross(e1);
  e1 = e1.normalize();
  e2 = dp.cross(e1);
  e2 = e2.normalize();
  for(int i = 0; i < N + 1; i++) {
    const double theta1 = 6.28 * (float)i / N;
    const double theta2 = 6.28 * (float)(i + 1) / N;
    const auto v1 = (e1 * std::cos(theta1) + e2 * std::sin(theta1));
    const auto v2 = (e1 * std::cos(theta2) + e2 * std::sin(theta2));
    AddTriangle(pos, pos + v2, pos + v1, col);
  }
}

void uiWindow::AddCone(const Vector3& pos, const Vector3& dir, float size, const Vector3b& col) {
  const auto dp = dir.normalize();
  constexpr int N = 25; // 分割数
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
    const auto v1 = (e1 * std::cos(theta1) + e2 * std::sin(theta1)) * size;
    const auto v2 = (e1 * std::cos(theta2) + e2 * std::sin(theta2)) * size;
    AddTriangle(pos + dp * size, pos + v2, pos + v1, col);
  }
  AddCircle(pos, dp * size, col);
}


void uiWindow::AddCross(const Vector3& pos, const int n, const Vector3b col) {
  const auto w = std::max<int>(n / 10, 2);
  AddLine(pos - Vector3(n, 0, 0), pos + Vector3(n, 0, 0), col, w);
  AddLine(pos - Vector3(0, n, 0), pos + Vector3(0, n, 0), col, w);
  AddLine(pos - Vector3(0, 0, n), pos + Vector3(0, 0, n), col, w);
}
void uiWindow::AddCoord(const Vector3& pos, const Vector3& axis, const int n) {
  const auto w = std::max<int>(n / 10, 2);
  AddLine(pos, pos + Vector3(n, 0, 0), {0, 0, 255}, w);
  AddLine(pos, pos + Vector3(0, n, 0), {0, 255, 0}, w);
  AddLine(pos, pos + Vector3(0, 0, n), {255, 0, 0}, w);
}

void uiWindow::AddGridXY(const Vector2 range, const int n, const Vector3b col) {
  const int w = std::max<int>(float(range[1] - range[0]) * 0.01, 2);
  for(int i = 0; i < n; i++) {
    const int p = (range[1] - range[0]) * i / n + range[0];
    const auto c = (i == 0 || i == n - 1) ? col : col * 0.8;
    AddLine({(double)p, range[0], 0}, {(double)p, range[1], 0}, c, w);
    AddLine({range[0], (double)p, 0}, {range[1], (double)p, 0}, c, w);
  }
}

void uiWindow::AddRotatedRectPosSize(const Vector2d& pos, const Vector2d& size, const double theta, const Vector3b& col) {
  const Mat2x2 rmat = {std::cos(theta), -std::sin(theta), std::sin(theta), std::cos(theta)};
  const auto hs = size / 2;
  AddQuad2D(pos + rmat * Vector2(hs[0], hs[1]), pos + rmat * Vector2(-hs[0], hs[1]), pos + rmat * Vector2(-hs[0], -hs[1]), pos + rmat * Vector2(hs[0], -hs[1]), col);
}

void uiWindow::AddRotatedRectPosSize(const Vector2d& pos, const Vector2d& size, const double theta, const Vector3b& col, const float width) {
  const Mat2x2 rmat = {std::cos(theta), -std::sin(theta), std::sin(theta), std::cos(theta)};
  const auto hs = size / 2;
  AddQuad2D(pos + rmat * Vector2(hs[0], hs[1]), pos + rmat * Vector2(-hs[0], hs[1]), pos + rmat * Vector2(-hs[0], -hs[1]), pos + rmat * Vector2(hs[0], -hs[1]), col, width);
}

void uiWindow::AddArrow2D(const Vector2d& from, const Vector2d& to, const Vector3b& col, const float width) {
  AddLine2D(from, to, col, width);
  const auto d = to - from;
  const double theta = std::atan2(d[1], d[0]);
  constexpr double dt = 0.5;
  constexpr int l = 50;
  const auto pa = Vector2d(std::cos(theta + dt), std::sin(theta + dt)) * l;
  const auto pb = Vector2d(std::cos(theta - dt), std::sin(theta - dt)) * l;
  AddLine2D(to - pa, to, col, width);
  AddLine2D(to - pb, to, col, width);
}

Vector2d uiWindow::get_text_size(const std::string& str, float size) const {
  int x = 0;
  int y = 0;
  const std::u32string u32str = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(str);
  const auto text_renderer = ::vkUI::Engine::getTextRendererPtr();
  const int spacing = getStyle()->TextSpacing;
  Vector2d whole_size{0, 0};
  if(u32str.size() == 0) return {0, 0};
  y += text_renderer->FindGlyph(u32str[0])->dHeight * size;
  for(int i = 0; i < u32str.size(); i++) {
    if((uiWchar)u32str[i] == '\n' && u32str.size() > i + 1) {
      x = 0;
      y = whole_size[1] + text_renderer->FindGlyph(u32str[i + 1])->dHeight * size + spacing;
      continue;
    }
    const auto glyph = text_renderer->FindGlyph(u32str[i]);
    const int y_tmp = y + (-glyph->dHeight + glyph->V1 - glyph->V0) * size;
    x += spacing + (glyph->U1 - glyph->U0) * size;
    whole_size = {std::max<int>(whole_size[0], x), std::max<int>(whole_size[1], y_tmp)};
  }
  return whole_size;
}

void uiWindow::AddStringBalloon(const std::string& str, const Vector3& pos, const float size, const Vector3b& col, const Vector3b& line) {
  /* _Mat<float, 4, 4> R1; */
  /* camera_position.getCameraProjMatrix(R1.value); */
  /* const _Mat<float, 4, 4> R2{ */
  /*   float(this->size[0])/2.0f, 0.0f, 0.0f, 0.0f, */
  /*   0.0f, float(this->size[1])/2.0f, 0.0f, 0.0f, */
  /*   0.0f, 0.0f, 1.0f, 0.0f, */
  /*   -1.0f, -1.0f, 0.0f, 1.0f */
  /* }; */
  /* auto projected = R1 * Vector4{pos[0], pos[1], pos[2], 1.0 }; */
  float R1[16];
  camera_position.getCameraProjMatrix(R1);
  float projected[4];
  projected[0] = R1[0] * pos[0] + R1[1] * pos[1] + R1[2] * pos[2] + R1[3] * 1;
  projected[1] = R1[4] * pos[0] + R1[5] * pos[1] + R1[6] * pos[2] + R1[7] * 1;
  projected[3] = R1[12] * pos[0] + R1[13] * pos[1] + R1[14] * pos[2] + R1[15] * 1;

  const auto to = Vector2d{(int)(projected[0] * this->size[0] / 2.0f / projected[3] + this->size[0] / 2.0f), (int)(projected[1] * this->size[1] / 2.0f / projected[3] + this->size[1] / 2.0f)};
  AddStringBalloon2D(str, to, size, col, line);
}


void uiWindow::AddStringBalloon2D(const std::string& str, const Vector2d& to, const float size, const Vector3b& col, const Vector3b& line) {
  const bool is_arrow_up = to[1] > 35; // 通常True
  const auto tsize = get_text_size(str, size);
  int x, y;
  constexpr int line_width = 2;
  constexpr int leader_len = 25;
  if(is_arrow_up) {
    AddLine2D(to, to + Vector2d(3, -5), line, line_width);
    AddLine2D(to, to + Vector2d(leader_len, -leader_len), line, line_width);
    AddLine2D(to + Vector2d(leader_len, -leader_len), to + Vector2d(leader_len + tsize[0] + 10, -leader_len), line, line_width);
    x = to[0] + leader_len + 5;
    y = to[1] - leader_len - tsize[1] - 10;
  } else {
    AddLine2D(to, to + Vector2d(3, 5), line, line_width);
    AddLine2D(to, to + Vector2d(leader_len, leader_len), line, line_width);
    AddLine2D(to + Vector2d(leader_len, leader_len), to + Vector2d(leader_len + tsize[0] + 10, leader_len), line, line_width);
    x = to[0] + leader_len + 5;
    y = to[1] + leader_len - tsize[1] - 10;
  }
  AddString2D(str, {x, y}, 1, col);
}

// ----------------------------------------------------------------
//     [ SECTION ] 2D Drawing Functions
// ----------------------------------------------------------------
void uiWindow::__AddPointSizeZero2D(const Vector2d& pos, const Vector3b& col) {
  const auto text_renderer = getTextRendererPtr();
  /* dd.vertices_ui.push_back(std::move(VertexUI(pos, col, text_renderer->TexUvWhitePixel))); */
  dd.add(pos, col, text_renderer->TexUvWhitePixel);
}

Vector2d uiWindow::AddString2D(const std::string& str, const Vector2d& pos, const float size, const Vector3b& col, const int xlim) {
  const std::u32string u32str = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(str);
  int x = pos[0];
  int y = pos[1];
  const auto text_renderer = ::vkUI::Engine::getTextRendererPtr();
  const int spacing = getStyle()->TextSpacing;
  Vector2d whole_size{0, 0};
  // const int w = text_renderer->TexWidth;
  // const int h = text_renderer->TexHeight;
  if(u32str.size() == 0) {
    return {0, 0};
  }

  y += text_renderer->FindGlyph(u32str[0])->dHeight * size;
  for(int i = 0; i < u32str.size(); i++) {
    const auto glyph = text_renderer->FindGlyph(u32str[i]);
    const int x2 = x + (glyph->U1 - glyph->U0) * size;
    if(((uiWchar)u32str[i] == '\n' || x2 - pos[0] > xlim) && u32str.size() > i + 1) {
      x = pos[0];
      y = pos[1] + whole_size[1] + text_renderer->FindGlyph(u32str[i + 1])->dHeight * size + spacing;
      continue;
    }
    const int y_tmp = y + (-glyph->dHeight + glyph->V1 - glyph->V0) * size;
    const int y1 = std::max<int>(0, y - glyph->dHeight * size);
    const int y2 = std::max<int>(0, y_tmp);
    AddQuad2D({x, y1}, {x, y2}, {x2, y2}, {x2, y1}, {glyph->U0, glyph->V0}, {glyph->U0, glyph->V1}, {glyph->U1, glyph->V1}, {glyph->U1, glyph->V0}, col);
    x += spacing + (glyph->U1 - glyph->U0) * size;

    whole_size = {std::max(whole_size[0], x - pos[0]), std::max(whole_size[1], y_tmp - pos[1])};
  }
  return whole_size;
}

Vector2d uiWindow::AddString2D(const std::string& str, const Vector2d& pos, const float size, const uiRect& clip_rect, const Vector3b& col) {
  const std::u32string u32str = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(str);
  int x = pos[0];
  int y = pos[1];
  const auto text_renderer = ::vkUI::Engine::getTextRendererPtr();
  const int spacing = getStyle()->TextSpacing;
  Vector2d whole_size{0, 0};
  // const int w = text_renderer->TexWidth;
  // const int h = text_renderer->TexHeight;
  if(u32str.size() == 0) {
    return {0, 0};
  }

  y += text_renderer->FindGlyph(u32str[0])->dHeight * size;
  for(int i = 0; i < u32str.size(); i++) {
    if((uiWchar)u32str[i] == '\n' && u32str.size() > i + 1) {
      x = pos[0];
      y = pos[1] + whole_size[1] + text_renderer->FindGlyph(u32str[i + 1])->dHeight * size + spacing;
      continue;
    }
    const auto glyph = text_renderer->FindGlyph(u32str[i]);
    const int y_tmp = y + (-glyph->dHeight + glyph->V1 - glyph->V0) * size;
    const int y1 = std::max<int>(0, y - glyph->dHeight * size);
    const int y2 = std::max<int>(0, y_tmp);
    const int x2 = x + (glyph->U1 - glyph->U0) * size;
    if(clip_rect.isContains(x, y1) && clip_rect.isContains(x2, y2)) {
      AddQuad2D({x, y1}, {x, y2}, {x2, y2}, {x2, y1}, {glyph->U0, glyph->V0}, {glyph->U0, glyph->V1}, {glyph->U1, glyph->V1}, {glyph->U1, glyph->V0}, col);
    }
    x += spacing + (glyph->U1 - glyph->U0) * size;

    whole_size = {std::max(whole_size[0], x - pos[0]), std::max(whole_size[1], y_tmp - pos[1])};
  }
  return whole_size;
}

void uiWindow::terminate() {
  glfwDestroyWindow(window);
  glfwSwapBuffers(window);
  glfwPollEvents();
}

} // namespace vkUI::Engine
