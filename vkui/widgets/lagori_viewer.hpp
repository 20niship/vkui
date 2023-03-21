
#pragma once
#include <functional>
#include <widget.hpp>
#include <engine.hpp>
#include <magi/cluster_manager.hpp>
#include <magi/cluster_manager_regiongrowing.hpp>
#include <magi/ball_tracker.hpp>

namespace vkUI{
// 表示するもの

class uiLagoriViewer : public uiWidget{
private:
  Magi::ClusterManager2 *manager{nullptr};
  Magi::ClusterManagerRegionGrowing *reg{nullptr};
  Magi::CamInfo *cam{nullptr};
  Magi::BallTracking *trc{nullptr};

  void render_field();
  void render_reg();
  void render_cam();
  void render_manager();
  void render_trc();

  inline Vector2d cvt_gpos(Vector2d g){return pos + cvt_gsize(g);}
  inline Vector2d cvt_gsize(Vector2d g){return Vector2d((double)g[0] * sx, (double)g[1] * sy);}
  inline Vector2d cvt_gpos(const int x, const int y){return cvt_gpos({x, y}); }
  inline Vector2d cvt_gsize(const int x, const int y){return cvt_gsize({x, y}); }

  double sx{1}, sy{1};
  uiVector<Vector2d> found_points;
  
public:
	uiLagoriViewer();
  void set_cluster_manager(Magi::ClusterManager2 *manager_){manager = manager_; }
  void set_cam(Magi::CamInfo *c){cam = c; }
  void set_cluster_reg(Magi::ClusterManagerRegionGrowing *reg_){ reg = reg_; }
  void set_ball_trackers(Magi::BallTracking *ptr){ trc = ptr; }
	void render() override;
	bool CallbackFunc(uiCallbackFlags flag, Vector2d vec2_1, int num_1, int num_2, const char **strings) override;
};

inline uiWidget *LagoriViewer(){ return (new uiLagoriViewer());}
} // namespace vkUI


