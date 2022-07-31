#pragma once
#include <cutil/vector.hpp>
#include <functional>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#define VK_ENGINE_MAX_FRAMES_IN_FLIGHT 2
#define VK_ENGINE_ENABLE_VALIDATION_LAYERS
#define VKUI_ENGINE_ENABLE_FPS_CALC
/* #define VKUI_ENGINE_USE_FLOAT_VERTEX */


namespace vkUI::Render {
class glWndRender;
}

namespace vkUI::Engine {


struct UniformData {
  float proj[16];
  float proj_uv[16];
  float texure_size[2];
};
struct DrawCmd {
  int start, size;
  int tex_id;
  int z_index;
  Vector2d scissor_top, scissor_btm;
};


struct vkVertex {
#ifdef VKUI_ENGINE_USE_FLOAT_VERTEX
  float pos[3];
  // float uv[2];
#else
  int16_t pos[3];
  // uint16_t uv[2];
#endif
  uint8_t col[3];
  static auto get_offset_pos() {
    return offsetof(vkVertex, pos);
  }
  static auto get_offset_col() {
    return offsetof(vkVertex, col);
  }

  vkVertex(const Vector3& _pos, const Vector3b& _col /*, const Vector2 _uv = {0.0f, 0.0f } */) {
    pos[0] = _pos[0];
    pos[1] = _pos[1];
    pos[2] = _pos[2];
    col[0] = _col[0];
    col[1] = _col[1];
    col[2] = _col[2];
    // uv[0]  = _uv[0];  uv[1]  = _uv[1];
  }
};

struct vkVertexUI {
  uint16_t pos[2];
  uint8_t col[3];
  uint16_t uv[2];

  static auto get_offset_pos() { return offsetof(vkVertexUI, pos); }
  static auto get_offset_col() { return offsetof(vkVertexUI, col); }
  static auto get_offset_uv() { return offsetof(vkVertexUI, uv); }

  vkVertexUI(const Vector2d& _pos, const Vector3b& _col, const Vector2d& _uv) {
    pos[0] = _pos[0];
    pos[1] = _pos[1];
    col[0] = _col[0];
    col[1] = _col[1];
    col[2] = _col[2];
    uv[0] = _uv[0];
    uv[1] = _uv[1];
  }
};
struct vkDrawData {
  UniformData uniform_data;
  uiVector<vkVertex> vertices;
  int _last_end, _tex_id, _z_index;
  std::vector<DrawCmd> drawlist;
  uiVector<vkVertexUI> vertices_ui;

  void set_tex_id(const int id) {
    if(id != _tex_id) push();
    _tex_id = id;
  }
  void set_z_index(const int z) {
    if(z != _z_index) push();
    _z_index = z;
  }
  void push() {
    DrawCmd d;
    d.start = _last_end;
    d.size = vertices_ui.size() - _last_end;
    d.tex_id = _tex_id;
    d.scissor_btm = {0, 0};
    d.scissor_top = {0, 0};
    d.z_index = _z_index;
    drawlist.push_back(d);
    _last_end = vertices_ui.size() - 1;
  }
  void clear() {
    drawlist.clear();
    vertices.clear();
    vertices_ui.clear();
  }
  auto sort() {
    std::sort(drawlist.begin(), drawlist.end(), [](const auto& x, const auto& y) { return x.z_index < y.z_index; });
  }
  inline void add(const Vector2d& p, const Vector3b& c, const Vector2d& uv) {
    vertices_ui.push_back(std::move(vkVertexUI{p, c, uv}));
  }
  auto get_n_vertices_ui() const { return vertices_ui.size(); }
  auto get_n_vertices() const { return vertices.size(); }
  auto get_n_commands() const { return drawlist.size(); }
};



#if  0
#ifndef GLuint
using GLuint = unsigned int;
#endif

struct glDrawData {
  uiVector<uint16_t> vertex_array;
  uiVector<uint8_t> col_array;
  uiVector<uint16_t> cord_array;

  uiVector<GLuint> cmds; // GLuint = unsigned intなので、正の数でなければならない
  /* GLFWwindow *_hWnd; */
  // (draw_order,   index,      vertex_num ) 　<- Polygon描画の時
  // (DRAW_IMAGE,   index,      texture_num)   <- Texture描画の時
  //　(DRAW_TEXT,   pos_index, string_index)  <- 文字描画の時
  //  (SET_SCISSOR, scissor_index, ??) <- glScissorをセット
  // draw_order = (GL_LINE_LOOP, GL_LINES, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN,
  //              DRAW_IMAGE, DRAW_TEXT, DRAW_TEXT_DISABLED, DRAW_TEXT_, SET_SCISSOR)

  /* const float CurveFineness; */
  float cliprect_left, cliprect_top, cliprect_bottom, cliprect_right;

  UniformData uniform_data;
  int _last_end, _tex_id, _z_index;
  std::vector<DrawCmd> drawlist;
  /*inline bool isAllCotainsCliprect(Vector2f pos, Vector2f size){ */
  /*	return cliprect_left <= pos.x && pos.x + size.x <= cliprect_right && */
  /*		   cliprect_top  <= pos.y && pos.y + size.y <= cliprect_bottom; */
  /*} */
  /*/1*! Outsideにある時（trueが返されたとき）はレンダリングをしない *1/ */
  /*inline bool isOutsideCliprect(Vector2f pos, Vector2f size){ */
  /*	return pos.x + size.x <= cliprect_left || cliprect_right  <= pos.x || */
  /*		   pos.y + size.y <= cliprect_top  || cliprect_bottom <= pos.y; */
  /*} */

  /* friend Render::glWndRender; */
public:
  /*/1*! ClipRectの内側でのみレンダリングを行い、外側にはクリップする *1/ */
  /*inline void setClipRect(Vector2f pos, Vector2f size){ */
  /*	cliprect_left = pos.x; cliprect_right  = pos.x + size.x; */
  /*	cliprect_top  = pos.y; cliprect_bottom = pos.y + size.y; */
  /*} */

  inline void add(const Vector2d& p, const Vector3b& c, const Vector2d& uv) {
    /* vertex_array.push_back((float)rand() / RAND_MAX); */
    /* vertex_array.push_back((float)rand() / RAND_MAX); */

    /* col_array.push_back((float)rand() / RAND_MAX); */
    /* col_array.push_back((float)rand() / RAND_MAX); */
    /* col_array.push_back((float)rand() / RAND_MAX); */

    /* cord_array.push_back((float)rand() / RAND_MAX); */
    /* cord_array.push_back((float)rand() / RAND_MAX); */

    vertex_array.push_back(p[0]);
    vertex_array.push_back(p[1]);
    col_array.push_back(c[0]);
    col_array.push_back(c[1]);
    col_array.push_back(c[2]);
    cord_array.push_back(uv[0]);
    cord_array.push_back(uv[1]);
  }
  void set_tex_id(const int id) {
    if(id != _tex_id) push();
    _tex_id = id;
  }
  void set_z_index(const int z) {
    if(z != _z_index) push();
    _z_index = z;
  }
  void push() {
    DrawCmd d;
    d.start = _last_end;
    d.size = vertex_array.size() - _last_end;
    d.tex_id = _tex_id;
    d.scissor_btm = {0, 0};
    d.scissor_top = {0, 0};
    d.z_index = _z_index;
    drawlist.push_back(d);
    _last_end = vertex_array.size() - 1;
  }
  void clear() {
    drawlist.clear();
    vertex_array.clear();
    col_array.clear();
    cord_array.clear();
  }
  auto sort() {
    std::sort(drawlist.begin(), drawlist.end(), [](const auto& x, const auto& y) { return x.z_index < y.z_index; });
  }

  auto get_n_vertices_ui() const { return vertex_array.size(); }
  auto get_n_commands() const { return drawlist.size(); }
};

#endif 

using glDrawData = vkDrawData;

} // namespace vkUI::Engine
