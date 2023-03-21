#include <drawdata.hpp>
#include <engine.hpp>

namespace vkUI::Render {
VertexUI::VertexUI(const Vector2d& _pos, const Vector3b& _col) {
  const auto whiteuv = vkUI::getWhitePixel();
  pos[0] = _pos[0];
  pos[1] = _pos[1];
  col[0] = _col[0];
  col[1] = _col[1];
  col[2] = _col[2];
  uv[0] = whiteuv[0];
  uv[1] = whiteuv[1];
}
} // namespace vkUI::Render
