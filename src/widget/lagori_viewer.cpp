#include "magi/utils.hpp"
#include "myvector.hpp"
#include <cmath>
#include <cstdlib>
#include <engine.hpp>
#include <string>
#include <type_traits>
#include <widgets/lagori_viewer.hpp>
#include <algorithm>
#include <vector>
#include <logger.hpp>
#include <tuple>
#include <vector>
#include <sstream>

namespace vkUI{
static std::vector<std::tuple<Vector2d, Vector2d, Vector3b>> field_pos = {
  {{0,0},{2500,12000}, {255,0,0}},
  {{2500,0},{7000,1200,},{0,255,0}},
  {{2500,1500},{9000,7000},{255,100,255}},
  {{9500,0},{2500,12000},{0,0,255}},
  {{3500,5000},{1000,2000},{200,200,200}},
  {{8500,5000},{1000,2000},{200,200,200}},
  {{2500, 10500},{7000,1500},{0,255,0}}
};


uiLagoriViewer::uiLagoriViewer(){ 
  flags.EnableAutoExpandX = 1;
  flags.EnableAutoExpandY = 1;
  flags.EnableAutoShrinkX = 1;
  flags.EnableAutoShrinkY = 1;
  impl_needCalcAlignment();
  needRendering(true);
}

void uiLagoriViewer::render_field(){
  auto wnd = Engine::getDrawingWindow();
  for(const auto &f: field_pos){
    const auto p = std::get<0>(f);
    const auto s = std::get<1>(f);
    const auto col = std::get<2>(f);
    wnd->AddRectPosSize(cvt_gpos(p), cvt_gsize(s), col / 3);
  }
}

void uiLagoriViewer::render_cam(){
  auto wnd = Engine::getDrawingWindow();
  const auto cam_pos = Vector2d(cam->robot_offset[0], cam->robot_offset[1]);
  const double theta = cam->robot_rotation;
  wnd->AddCross2D(pos + cam_pos, 10, {255,255,255});
  wnd->AddString2D("camera", cvt_gpos(cam_pos) + Vector2d{0,20}, 1.0);
  wnd->AddRotatedRectPosSize(cvt_gpos(cam_pos), cvt_gsize(1000, 1000), theta, {255, 255, 255}, 2);
  const auto tt = Vector2d(60.0f * std::cos(theta), 60.0f * std::sin(theta));
  const auto tt2 = Vector2d(60.0f * std::sin(theta), -60.0f * std::cos(theta));
  wnd->AddLine2D(cvt_gpos(cam_pos), cvt_gpos(cam_pos)+tt, {255, 30, 30}, 2);
  wnd->AddLine2D(cvt_gpos(cam_pos), cvt_gpos(cam_pos)+tt2, {30, 255, 30}, 2);
}

void uiLagoriViewer::render_reg(){
  auto wnd = Engine::getDrawingWindow();
  const auto clusters = reg->clusters;
  for(int i=0; i<reg->getRegionNum(); i++){
    const auto r = clusters[i]->fit_result;
    if(r.type == Magi::ClusterType::Background) continue;
    if(r.type == Magi::ClusterType::Daiza) continue;
    if(r.type == Magi::ClusterType::Invalid) continue;
    if(r.type == Magi::ClusterType::NotDefined) continue;

    const Vector2d p2 = {
      (int)(r.xyz[0] * sx), (int)(r.xyz[1] * sx),
    };
    const int s = sx * (
      (r.type == Magi::ClusterType::LagoriUpSurface) ? r.size[2] : r.size[0]
    );
    const Vector3b col = (
      (r.type == Magi::ClusterType::BlueLagori) ? Vector3b(0,0,255) : 
      (r.type == Magi::ClusterType::RedLagori) ? Vector3b(255,0,9) : Vector3b(255, 255, 255) 
    );
    wnd->AddCircle2D(pos + p2, s, col, 2);
  }
}

void uiLagoriViewer::render_manager(){
  auto wnd = Engine::getDrawingWindow();
  for(const auto &o : manager->objects){
    const auto p2 = cvt_gpos(o.pos_raw[0], o.pos_raw[1]);
    const int id = o.lagori_id;
    const int s = sx * (id * 75 + 200);
    wnd->AddCircle2D(pos + p2, s, ((id % 2 == 0) ? Vector3b{255,0,0} : Vector3b{0,0,255}), 2);
  }
}

void uiLagoriViewer::render_trc(){
  const auto active = trc->active();
  const auto points = trc->get_n_point();
  const auto [p1, s1] = trc->get_volume_3d();
  const auto time = trc->get_tracking_time_ms();
  const auto r = trc->get_roi();
  const auto p = trc->get_current_pos();
  const auto roi = trc->get_roi();
  const auto roi3d = trc->get_roi_3d();

  auto wnd = Engine::getDrawingWindow();

  const auto tmp_ball_size = 200.0f * sx;
  const auto col = active ? Vector3b(255, 255, 0) : Vector3b(100, 100, 100);
  wnd->AddRectPosSize(cvt_gpos(p1[0], p1[1]), cvt_gsize(s1[0], s1[1]), col, 3.0f);
  wnd->AddCircle2D(cvt_gpos(p[0], p[1]), tmp_ball_size, {255, 0, 255} );
  std::string str = "ball : " + std::to_string(time);
  wnd->AddStringBalloon2D(str, cvt_gpos(p1[0], p1[1]), 1, {255,255,255});

  // 軌道を描画
  const auto vxy = trc->trc.fitting_xy.result;
  const Vector2d pos1 = {0, (int)vxy[1]};
  const Vector2d pos2 = {12000, (int)(vxy[0]*12000.0f + vxy[1])};
  wnd->AddLine2D(cvt_gpos(pos1), cvt_gpos(pos2), {200, 200, 200}, 2);

  // BHの動かす向きを描画
  const auto avoid_r = trc->get_params().avoid_min_radius * sx;
  wnd->AddCircle2D(cvt_gpos(cam->robot_offset[0],cam->robot_offset[1]), avoid_r, {255, 0, 255}, 2);
  {
    const Vector2 p_ = { 
      std::sin(cam->robot_rotation),
      std::cos(cam->robot_rotation), 
    };
    const Vector2 cam_pos = {cam->robot_offset[0],cam->robot_offset[1]};
    const auto _tmp = cam_pos + p_ * trc->get_slider_goto();
    const bool should_move = trc->should_avoid_bh();
    const auto col = should_move ? Vector3b(255, 0, 255) : Vector3b(90,90,90);
    wnd->AddArrow2D(cvt_gpos(cam_pos[0], cam_pos[1]), cvt_gpos(_tmp[0], _tmp[1]), col, 3);
  }

  found_points.reserve(1000);
  if(active){
    found_points.push_back(cvt_gpos(p[0], p[1]));
  }else{
    found_points.clear();
  }
  for(const auto p_ : found_points) wnd->AddPlus2D(p_, 10, {255, 90, 90});

  if(!active) return;
  std::stringstream ss;
  ss <<" ----  Ball Tracking Result ----" << std::endl;
  ss << "tracking : " << (active ? "true" :"false" ) << std::endl; 
  ss << "n points : " << points << std::endl; 
  ss << "dt ms    : " << time << std::endl; 
  ss << "center   : " << p << std::endl; 
  ss << "area     : " << roi.get_area() << std::endl; 
  ss << "are (3d) : " << roi3d.get_volume() << std::endl; 
  ss << "W(3d)    : " << roi3d.width() << std::endl; 
  ss << "H(3d)    : " << roi3d.height() << std::endl; 
  ss << "D(3d)    : " << roi3d.depth() << std::endl; 
  ss << "UVcenter : " << roi.center()[0] << ", " << roi.center()[1] << std::endl;

  const auto strsize = wnd->get_text_size(ss.str(), 1);
  const Vector2d str_pos = this->pos + this->size - strsize - 30; 
  wnd->AddRectPosSize(str_pos, strsize, {40, 40, 80});
  wnd->AddString2D(ss.str(), str_pos, 1, {255, 255, 0});
  wnd->AddString2D("BALL DETECTED!!", str_pos - Vector2d{-30, 50}, 2, {255, 50, 50});
}

void uiLagoriViewer::render(){
  sx = (double)size[0] / 12000;
  sy = (double)size[1] / 12000;
  sx = sy = std::min(sx, sy);

  render_field();
  if(cam != nullptr) render_cam();
  if(reg != nullptr) render_reg();
  if(manager != nullptr) render_manager();
  if(trc != nullptr) render_trc();
}

bool uiLagoriViewer::CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings){ 
   return true;
}

} 
