#pragma once
#include <cutil/vector.hpp>
#include <functional>
#include <stdio.h>
#include <vector>

#define VK_ENGINE_MAX_FRAMES_IN_FLIGHT 2
#define VK_ENGINE_ENABLE_VALIDATION_LAYERS
#define VKUI_ENGINE_ENABLE_FPS_CALC
/* #define VKUI_ENGINE_USE_FLOAT_VERTEX */

namespace vkUI {
using namespace Cutil;
struct uiRect {
  uiRect() {
    posX = 0;
    posY = 0;
    width = 0;
    height = 0;
  }
  uiRect(float posX_, float posY_, float width_, float height_) {
    posX = posX_;
    posY = posY_;
    width = width_;
    height = height_;
  }
  template <typename U> uiRect(_Vec<U, 2> pos, _Vec<U, 2> size) {
    posX = pos[0];
    posY = pos[1];
    width = size[0];
    height = size[1];
  }

  float posX, posY, width, height;
  inline float right() { return posX + width; }
  inline float bottom() { return posY + height; }
  inline bool isContains(const int x, const int y) const { return (x >= posX) && (x <= posX + width) && (y >= posY) && (y <= posY + height); }
  template <typename U> inline bool isContains(const _Vec<U, 2> other) const { return isContains(other[0], other[1]); }

  inline bool isNoContains(uiRect outside) { return (outside.posX > right()) || (outside.right() < posX) || (outside.posY > bottom()) || (outside.bottom() < posY); }

  inline Vector2 getPos() { return {posX, posY}; }
  inline Vector2 getSize() { return {width, height}; }
};

} // namespace vkUI

namespace vkUI::Render {
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

struct Vertex {
#ifdef VKUI_ENGINE_USE_FLOAT_VERTEX
  float pos[3];
  // float uv[2];
#else
  int16_t pos[3];
  // uint16_t uv[2];
#endif
  uint8_t col[3];
  static auto get_offset_pos() {
    return offsetof(Vertex, pos);
  }
  static auto get_offset_col() {
    return offsetof(Vertex, col);
  }

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
  uint16_t pos[2];
  uint8_t col[3];
  uint16_t uv[2];

  static auto get_offset_pos() { return offsetof(VertexUI, pos); }
  static auto get_offset_col() { return offsetof(VertexUI, col); }
  static auto get_offset_uv() { return offsetof(VertexUI, uv); }

  VertexUI(const Vector2d& _pos, const Vector3b& _col, const Vector2d& _uv) {
    pos[0] = _pos[0];
    pos[1] = _pos[1];
    col[0] = _col[0];
    col[1] = _col[1];
    col[2] = _col[2];
    uv[0] = _uv[0];
    uv[1] = _uv[1];
  }

  VertexUI(const Vector2d& _pos, const Vector3b& _col);
};

struct DrawData {
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
  void clear() {
    drawlist.clear();
    vertices.clear();
    vertices_ui.clear();
  }
  auto sort() {
    std::sort(drawlist.begin(), drawlist.end(), [](const auto& x, const auto& y) { return x.z_index < y.z_index; });
  }
  auto get_n_vertices_ui() const { return vertices_ui.size(); }
  auto get_n_vertices() const { return vertices.size(); }
  auto get_n_commands() const { return drawlist.size(); }


  // -------- drawing commands -----------------
  void line(const Vector3& pos1, const Vector3& pos2, const Vector3b& col1, const Vector3b& col2, const float width = 1.0);
  void line(const Vector3& pos1, const Vector3& pos2, const Vector3b& col, const float width = 1.0);

  void point(const Vector3& pos, const Vector3b& col, const double size = 1.0);
  void point(const Vector3& pos, const Vector3b& col, const Vector3b& col2, const double size = 1.0);
  void sphere_20(const Vector3& pos, const float size, const Vector3b& col);

  void arrow(const Vector3& pos1, const Vector3& pos2, const Vector3b& col, float width = 1.0);
  void arrow_to(const Vector3& pos, const Vector3& dir, const Vector3b& col, float width);

  void cube(const Vector3& pos, const Vector3& whd, const Vector3b& col);
  void cube(const Vector3& pos, const float size, const Vector3b& col);
  void cube(const Vector3& pos, const Vector3& size, const Vector3& pry, const Vector3b& col);

  void cube(const Vector3& pos, const Vector3& whd, const Vector3b& col, const int width);
  void cube(const Vector3& pos, const float size, const Vector3b& col, const int width);
  void cube(const Vector3& pos, const Vector3& size, const Vector3& pry, const Vector3b& col, const int width);

  void plane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col);
  void plane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col, int width);

  void circle(const Vector3& pos, const Vector3& normal, const Vector3b& col);
  void circle(const Vector3& pos, const Vector3& normal, const Vector3b& col, const int width);

  void cone(const Vector3& pos, const Vector3& dir, float size, const Vector3b& col);
  void baloon(const std::string& str, const Vector3& pos, const float size, const Vector3b& col = {255, 255, 255}, const Vector3b& line = {255, 255, 255});

  void baloon(const std::string& str, const Vector2d& pos, const float size, const Vector3b& col = {255, 255, 255}, const Vector3b& line = {255, 255, 255});
  void gridxy(const Vector2 range, const int n = 10, const Vector3b col = {255, 255, 0});
  void cross(const Vector3& pos, const int n = 10, const Vector3b col = {255, 255, 0});
  void coord(const Vector3& pos, const Vector3& axis = {0, 0, 1}, const int n = 10);

  Vector2d get_text_size(const std::string& str, float size) const;
#define VKUI_USE_CLIPPING_RECT // TODO: 最終的にはこれなしで動くようにする
  // [ SECTION ] --------  2D Rendering functions ----------
  // VKUI_USE_CLIPPING_RECTがdefineされているときはクリッピングする
  // TODO: stackにしてpop/pushするべきかも
  void __AddPointSizeZero2D(const Vector2d& pos, const Vector3b& col);
  inline void __AddPointSizeZero2D(const Vector2d& pos, const Vector3b& col, const Vector2d& uv) {
    vertices_ui.push_back(std::move(VertexUI(pos, col, uv)));
  }

  inline void triangle(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3, const Vector2d& uv1, const Vector2d& uv2,
                       const Vector2d& uv3) {
    vertices_ui.push_back(std::move(VertexUI(pos1, col1, uv1)));
    vertices_ui.push_back(std::move(VertexUI(pos3, col3, uv3)));
    vertices_ui.push_back(std::move(VertexUI(pos2, col2, uv2)));
  }
  inline void triangle(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3) {
    vertices_ui.push_back(std::move(VertexUI(pos1, col1)));
    vertices_ui.push_back(std::move(VertexUI(pos3, col3)));
    vertices_ui.push_back(std::move(VertexUI(pos2, col2)));
  }
  // TODO: クリッピングする実装
  inline void triangle(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector2d uv1, const Vector2d& uv2, const Vector2d& uv3, const Vector3b& col) {
    __AddPointSizeZero2D(pos1, col, uv1);
    __AddPointSizeZero2D(pos3, col, uv3);
    __AddPointSizeZero2D(pos2, col, uv2);
  }
  inline void triangle(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector3b& col) {
    __AddPointSizeZero2D(pos1, col);
    __AddPointSizeZero2D(pos3, col);
    __AddPointSizeZero2D(pos2, col);
  }

  inline void quad(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector2d& pos4, const Vector2d& uv1, const Vector2d& uv2, const Vector2d& uv3, const Vector2d& uv4,
                   const Vector3b& col) {
    triangle(pos1, pos2, pos3, uv1, uv2, uv3, col);
    triangle(pos1, pos3, pos4, uv1, uv3, uv4, col);
  }
  inline void quad(const Vector2d& p1, const Vector2d& p2, const Vector2d& p3, const Vector2d& p4, const Vector3b& col) {
    triangle(p1, p2, p3, col);
    triangle(p1, p3, p4, col);
  }

  inline void rectTB(const Vector2d& top, const Vector2d& btm, const Vector3b& col) {
    const Vector2d pos[4] = {
      top,
      Vector2d(top[0], btm[1]),
      btm,
      Vector2d(btm[0], top[1]),
    };
    triangle(pos[0], pos[1], pos[2], col);
    triangle(pos[0], pos[2], pos[3], col);
  }

  inline void rectTB(const Vector2d& top, const Vector2d& btm, const Vector3b& col, const float width) {
    const Vector2d pos[] = {
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
  inline void rectPS(const Vector2d& pos, const Vector2d& size, const Vector3b& col) {
    rectTB(pos, pos + size, col);
  }
  inline void rectPS(const Vector2d& p, const Vector2d& size, const Vector2d& ui1, const Vector2d& ui2, const Vector3b& col = {255, 255, 255}) {
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

    triangle(pos[0], pos[1], pos[2], col, col, col, upos[0], upos[1], upos[2]);
    triangle(pos[0], pos[2], pos[3], col, col, col, upos[0], upos[2], upos[3]);
  }

  inline void rectPS(const Vector2d& p, const Vector2d& size, const Vector3b& col1, const Vector3b& col2, const Vector3b& col3, const Vector3b& col4) {
    const auto top = p;
    const auto btm = p + size;
    const Vector2d pos[4] = {
      top,
      Vector2d(top[0], btm[1]),
      btm,
      Vector2d(btm[0], top[1]),
    };
    triangle(pos[0], pos[1], pos[2], col1, col2, col3);
    triangle(pos[0], pos[2], pos[3], col1, col3, col4);
  }

  void rotated_rectPS(const Vector2d& pos, const Vector2d& size, const double theta, const Vector3b& col);
  void rotated_rectPS(const Vector2d& pos, const Vector2d& size, const double theta, const Vector3b& col, const float width = 2.0f);
  inline void rectPS(const Vector2d& pos, const Vector2d& size, const Vector3b& col, float width) {
    rectTB(pos, pos + size, col, width);
  }
  inline void AddLine2D(const Vector2d& pos1, const Vector2d& pos2, const Vector3b& col1, const Vector3b& col2, const float width = 1.0) {
    // const auto dpd = pos2 - pos1;
    // const auto dp = Vector2{ (double)dpd[0], (double)dpd[1] } / dpd.norm();
    const auto dp = pos2 - pos1;
    const Vector2d e = {(int)(dp[1] * width / dp.norm()), -(int)(dp[0] * width / dp.norm())};
    triangle(pos1, pos2, pos2 + e, col1, col2, col2);
    triangle(pos1, pos2 + e, pos2, col1, col2, col2);
    triangle(pos1, pos2 + e, pos1 + e, col1, col2, col1);
    triangle(pos1, pos1 + e, pos2 + e, col1, col2, col1);
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

  inline void AddQuad2D(const Vector2d& pos1, const Vector2d& pos2, const Vector2d& pos3, const Vector2d& pos4, const Vector3b& col) {
    triangle(pos1, pos2, pos3, col);
    triangle(pos2, pos3, pos4, col);
  }

  void AddCheck2D(const Vector2d& pos, const int size, const Vector3b& col, float line_width) {
    AddLine2D(pos + Vector2({(float)size * 0.1, (float)size * 0.5}), pos + Vector2{(float)size * 0.5, (float)size * 0.9}, col, line_width);
    AddLine2D(pos + Vector2({(float)size * 0.5, (float)size * 0.9}), pos + Vector2{(float)size * 0.9, (float)size * 0.1}, col, line_width);
  }

  void AddArrowDown2D(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    triangle(pos, pos + Vector2d{size / 2, h}, pos + Vector2d{size, 0}, col);
  }

  void AddArrowUp2D(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    triangle(pos + Vector2d{0, h}, pos + Vector2d{size, h}, pos + Vector2d{size / 2, 0}, col);
  }

  void AddArrowLeft2D(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    triangle(pos + Vector2d{0, size / 2}, pos + Vector2d{h, size}, pos + Vector2d{h, 0}, col);
  }

  void AddArrowRight2D(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    triangle(pos + Vector2d{0, size}, pos + Vector2d{h, size}, pos + Vector2d{h, 0}, col);
  }
  void AddArrowRight2D2(const Vector2d& pos, const int size, const Vector3b& col) {
    const int h = (float)size * 0.85f;
    triangle(pos, pos + Vector2d{0, size}, pos + Vector2d{h, size / 2}, col);
  }

  void AddCrossButton(const Vector2d& pos, const int size, const Vector3b& bg_col, const Vector3b& line_col, const Vector3b& cross_col) {
    rectTB(pos, pos + size, bg_col);
    rectTB(pos, pos + size, line_col, 1);
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
    AddQuad2D(
        center + Vector2d{hs, 0}, 
        center + Vector2d{0, hs}, 
        center - Vector2d{hs, 0}, 
        center - Vector2d{0, hs}, 
        col);
  }

  void AddCircle2D(const Vector2d& pos, const int size, const Vector3b col) {
    if(size < 15) {
      constexpr double s45 = 0.707090402;
      const Vector2 p[8] = {{0, 1}, {s45, s45}, {1, 0}, {s45, -s45}, {0, -1}, {-s45, -s45}, {-1, 0}, {-s45, s45}};
      for(int i = 0; i < 6; i++) triangle(pos + p[0] * size, pos + p[1 + i] * size, pos + p[i + 2] * size, col);
    } else {
      constexpr double dth = 6.28 / 20.0f;
      for(int i = 0; i < 20; i++) {
        const auto p1 = pos + Vector2d(std::cos(dth * i) * size, std::sin(dth * i) * size);
        const auto p2 = pos + Vector2d(std::cos(dth * (i + 1)) * size, std::sin(dth * (i + 1)) * size);
        triangle(pos, p1, p2, col);
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
  Vector2d AddString2D(const std::string& str, const Vector2d& pos, const float size, const Vector3b& col = {255, 255, 255}, const int xlim = std::numeric_limits<int>::max());
  Vector2d AddString2D(const std::string& str, const Vector2d& pos, const float size, const uiRect& clip_rect, const Vector3b& col = {255, 255, 255});
};

} // namespace vkUI::Render
