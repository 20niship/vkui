#pragma once

#include <cutil/vector.hpp>
#include <functional>
#include <vector>


#define VK_ENGINE_MAX_FRAMES_IN_FLIGHT 2
#define VK_ENGINE_ENABLE_VALIDATION_LAYERS
#define VKUI_ENGINE_ENABLE_FPS_CALC
#define VKUI_ENGINE_USE_FLOAT_VERTEX


namespace vkUI::Engine {

struct Vertex {
#ifdef VKUI_ENGINE_USE_FLOAT_VERTEX
  float pos[3];
  // float uv[2];
#else
  int16_t pos[3];
  // uint16_t uv[2];
#endif
  uint8_t col[3];
  static auto get_offset_pos(){ return offsetof(Vertex, pos); }
  static auto get_offset_col(){ return offsetof(Vertex, col); }

  Vertex(const Vector3& _pos, const Vector3b& _col /*, const Vector2 _uv = {0.0f, 0.0f } */) {
    pos[0] = _pos[0];
    pos[1] = _pos[1];
    pos[2] = _pos[2];
    col[0] = _col[0];
    col[1] = _col[1];
    col[2] = _col[2];
    // uv[0]  = _uv[0];  uv[1]  = _uv[1];
  }
};

struct VertexUI {
  int16_t pos[2];
  uint8_t col[3];
  uint16_t uv[2];

  static auto get_offset_pos(){ return offsetof(VertexUI, pos); }
  static auto get_offset_col(){ return offsetof(VertexUI, col); }
  static auto get_offset_uv(){ return offsetof(VertexUI, uv); }

  template <typename T> VertexUI(const _Vec<T, 2>& _pos, const Vector3b& _col, const _Vec<T, 2>& _uv) {
    pos[0] = _pos[0];
    pos[1] = _pos[1];
    col[0] = _col[0];
    col[1] = _col[1];
    col[2] = _col[2];
    uv[0] = _uv[0];
    uv[1] = _uv[1];
  }
};
struct DrawData {
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

  UniformData uniform_data;
  uiVector<Vertex> vertices;
  int _last_end, _tex_id, _z_index;
  std::vector<DrawCmd> drawlist;
  uiVector<VertexUI> vertices_ui;


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
  void clear() { drawlist.clear(); }
  auto sort() {
    std::sort(drawlist.begin(), drawlist.end(), [](const auto& x, const auto& y) { return x.z_index < y.z_index; });
  }
};
} // namespace vkUI::Engine
