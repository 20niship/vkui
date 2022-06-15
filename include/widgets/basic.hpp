#include "widget.hpp"
#include "engine.hpp"

namespace vkUI{

class uiCube : public uiWidget{
private:
    WidgetDrawTypes drawtype;
    Vector3 pos, size, pry;
    Vector3b col;
public:
    uiCube(Vector3 _pos, Vector3 _size)
        { pos = _pos; size = _size; pry={0,0,0}; col = {255, 255, 255}; drawtype = WidgetDrawTypes::DrawAsSurface; }
    uiCube(Vector3 _pos, Vector3 _size, Vector3b _col)
        { pos = _pos; size = _size; pry={0,0,0};  col = _col; drawtype = WidgetDrawTypes::DrawAsSurface; }
    uiCube(Vector3 _pos, Vector3 _size, Vector3 _pry, Vector3b _col)
        { pos = _pos; size = _size; pry = _pry;  col = _col; drawtype = WidgetDrawTypes::DrawAsSurface; }
    void setRotation(const double p,  const double r, const double y){ pry = {p, r, y}; }
    void setDrawType(const WidgetDrawTypes & t){drawtype = t;}
	bool CallbackFunc([[maybe_unused]]uiCallbackFlags flag, [[maybe_unused]]Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings) override{ return true; };
	void render()override{
        const auto hWnd = ::vkUI::Engine::getDrawingWindow();
        
        const Mat3x3 rot0{
            1, 0,                0, 
            0, std::cos(pry[0]), -std::sin(pry[0]),
            0, std::sin(pry[0]), std::cos(pry[0])
        };
        const Mat3x3 rot1{
            std::cos(pry[1]), 0, std::sin(pry[1]),
            0,  1,  0,
            -std::sin(pry[1]), 0, std::cos(pry[1])
        };
        const Mat3x3 rot2{
            std::cos(pry[2]), -std::sin(pry[2]), 0, 
            std::sin(pry[2]), std::cos(pry[2]), 0, 
            0, 0, 1, 
        };
        const auto rot = rot0 * rot1 * rot2;
        const auto half_size = size/2;
        Vector3 points[8] = {
            pos + rot * Vector3(-half_size[0], -half_size[1], -half_size[2]),
            pos + rot * Vector3(-half_size[0], -half_size[1],  half_size[2]),
            pos + rot * Vector3(-half_size[0],  half_size[1],  half_size[2]),
            pos + rot * Vector3(-half_size[0],  half_size[1], -half_size[2]),
            pos + rot * Vector3( half_size[0], -half_size[1], -half_size[2]),
            pos + rot * Vector3( half_size[0], -half_size[1],  half_size[2]),
            pos + rot * Vector3( half_size[0],  half_size[1],  half_size[2]),
            pos + rot * Vector3( half_size[0],  half_size[1], -half_size[2]),
        };

        switch (drawtype){
        case WidgetDrawTypes::DrawAsSurface:
        {
            hWnd->AddQuad(points[0], points[1], points[2], points[3], {255, 0, 0});
            hWnd->AddQuad(points[3], points[2], points[6], points[7], {0, 255, 0});
            hWnd->AddQuad(points[7], points[6], points[5], points[4], {0, 0, 255});
            hWnd->AddQuad(points[4], points[5], points[1], points[0], {255, 255, 0});
            hWnd->AddQuad(points[0], points[3], points[7], points[4], {255, 0, 255});
            hWnd->AddQuad(points[5], points[6], points[2], points[1],{0, 255, 255});
            break;
        }

        case WidgetDrawTypes::DrawAsLines:
        {
            const float width = 5.0;
            hWnd->AddLine(points[0], points[1], {255, 255, 0}, width);
            hWnd->AddLine(points[1], points[2], {255, 255, 0}, width);
            hWnd->AddLine(points[2], points[3], {255, 255, 0}, width);
            hWnd->AddLine(points[3], points[0], {255, 255, 0}, width);
            hWnd->AddLine(points[4], points[5], {255, 255, 0}, width);
            hWnd->AddLine(points[5], points[6], {255, 255, 0}, width);
            hWnd->AddLine(points[6], points[7], {255, 255, 0}, width);
            hWnd->AddLine(points[0], points[4], {255, 255, 0}, width);
            hWnd->AddLine(points[1], points[5], {255, 255, 0}, width);
            hWnd->AddLine(points[2], points[6], {255, 255, 0}, width);
            hWnd->AddLine(points[3], points[7], {255, 255, 0}, width);
        }
        default:
            break;
        }
    }
};

#if 0

template<typename T>
class uiPointCloud : public uiWidget{
private:
    T *cloud;
    size_t size;
    Vector3b col;
    float display_size;
public:
    uiPointCloud(const T *_cloud, size_t _size, Vector3b _col = {255, 255, 255}, float _display_size = 1.0)
        { cloud = _cloud; size = _size; col = _col; display_size = _display_size;}
    void setColor(const Vector3b _col){ col = _col; }
	bool CallbackFunc([[maybe_unused]]uiCallbackFlags flag, [[maybe_unused]]Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings) override{ return true; };
    void setSize(size_t _size){size = _size; }
	void render(){
        const auto hWnd = ::vkUI::Engine::getDrawingWindow();
        for(size_t i=0; i<size; i++){ hWnd->AddPoint(cloud[i], col, display_size); }
    }
};

template <>
class uiPointCloud<Vector3> : public uiWidget{
private:
    Vector3 *cloud;
    size_t size;
    Vector3b col;
    float display_size;
public:
    uiPointCloud(Vector3 *_cloud, size_t _size, Vector3b _col = {255, 255, 255}, float _display_size = 1.0)
        { cloud = _cloud; size = _size; col = _col; display_size = _display_size;}
    void setColor(const Vector3b _col){ col = _col; }
    void setSize(size_t _size){size = _size; }
    void setDisplaySize(float _size){display_size = _size; }
    void setCloud(Vector3 *_cloud){ cloud = _cloud; }
	bool CallbackFunc([[maybe_unused]]uiCallbackFlags flag, [[maybe_unused]]Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings) override{ return true; };
	void render()override{
        const auto hWnd = ::vkUI::Engine::getDrawingWindow();
        auto *vertex_ptr = hWnd->getVertexPtr();
        const size_t start_idx = vertex_ptr->size() ;
        vertex_ptr->resize(start_idx + 3*size+3);
        
        const auto U =  hWnd->getCameraTarget() - hWnd->getCameraPos();
        const double ee = U[0]+U[1]+U[2];

        if(ee > 0){
            // #pragma omp parallel for
            for(size_t i=0; i<size; i++){ 
                (*vertex_ptr)[start_idx + i*3  ].pos[0] = cloud[i][0];
                (*vertex_ptr)[start_idx + i*3  ].pos[1] = cloud[i][1];
                (*vertex_ptr)[start_idx + i*3  ].pos[2] = cloud[i][2]+display_size;

                (*vertex_ptr)[start_idx + i*3+1].pos[0] = cloud[i][0];
                (*vertex_ptr)[start_idx + i*3+1].pos[1] = cloud[i][1]+display_size;
                (*vertex_ptr)[start_idx + i*3+1].pos[2] = cloud[i][2];

                (*vertex_ptr)[start_idx + i*3+2].pos[0] = cloud[i][0]+display_size;
                (*vertex_ptr)[start_idx + i*3+2].pos[1] = cloud[i][1];
                (*vertex_ptr)[start_idx + i*3+2].pos[2] = cloud[i][2];
                
                (*vertex_ptr)[start_idx + i*3  ].col[0] = col[0]; (*vertex_ptr)[start_idx + i*3  ].col[1] = col[1]; (*vertex_ptr)[start_idx + i*3  ].col[2] = col[2];
                (*vertex_ptr)[start_idx + i*3+1].col[0] = col[0]; (*vertex_ptr)[start_idx + i*3+1].col[1] = col[1]; (*vertex_ptr)[start_idx + i*3+1].col[2] = col[2];
                (*vertex_ptr)[start_idx + i*3+2].col[0] = col[0]; (*vertex_ptr)[start_idx + i*3+2].col[1] = col[1]; (*vertex_ptr)[start_idx + i*3+2].col[2] = col[2];
                // (*vertex_ptr)[start_idx + i*3  ] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(0.0f, 0.0f, display_size), col);
                // (*vertex_ptr)[start_idx + i*3+1] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(display_size, 0.0f, 0.0f), col);
                // (*vertex_ptr)[start_idx + i*3+2] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(0.0f, display_size, 0.0f), col);
            }
        }else{
            // #pragma omp parallel for
            for(size_t i=0; i<size; i++){ 
                (*vertex_ptr)[start_idx + i*3  ].pos[0] = cloud[i][0]+display_size;
                (*vertex_ptr)[start_idx + i*3  ].pos[1] = cloud[i][1];
                (*vertex_ptr)[start_idx + i*3  ].pos[2] = cloud[i][2];

                (*vertex_ptr)[start_idx + i*3+1].pos[0] = cloud[i][0];
                (*vertex_ptr)[start_idx + i*3+1].pos[1] = cloud[i][1]+display_size;
                (*vertex_ptr)[start_idx + i*3+1].pos[2] = cloud[i][2];

                (*vertex_ptr)[start_idx + i*3+2].pos[0] = cloud[i][0];
                (*vertex_ptr)[start_idx + i*3+2].pos[1] = cloud[i][1];
                (*vertex_ptr)[start_idx + i*3+2].pos[2] = cloud[i][2]+display_size;
                
                (*vertex_ptr)[start_idx + i*3  ].col[0] = col[0]; (*vertex_ptr)[start_idx + i*3  ].col[1] = col[1]; (*vertex_ptr)[start_idx + i*3  ].col[2] = col[2];
                (*vertex_ptr)[start_idx + i*3+1].col[0] = col[0]; (*vertex_ptr)[start_idx + i*3+1].col[1] = col[1]; (*vertex_ptr)[start_idx + i*3+1].col[2] = col[2];
                (*vertex_ptr)[start_idx + i*3+2].col[0] = col[0]; (*vertex_ptr)[start_idx + i*3+2].col[1] = col[1]; (*vertex_ptr)[start_idx + i*3+2].col[2] = col[2];
                // (*vertex_ptr)[start_idx + i*3  ] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(0.0f, 0.0f, display_size), col);
                // (*vertex_ptr)[start_idx + i*3+1] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(display_size, 0.0f, 0.0f), col);
                // (*vertex_ptr)[start_idx + i*3+2] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(0.0f, display_size, 0.0f), col);
            }
        }

    }
};

template <>
class uiPointCloud<::Magi::Point> : public uiWidget{
private:
    ::Magi::Point *cloud;
    size_t size;
    float display_size;
public:
    uiPointCloud(::Magi::Point *_cloud, size_t _size, float _display_size = 1.0)
        { cloud = _cloud; size = _size; display_size = _display_size;}
    void setSize(size_t _size){size = _size; }
	bool CallbackFunc([[maybe_unused]]uiCallbackFlags flag, [[maybe_unused]]Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings) override{ return true; };

#if 0
	void render(){
        const auto hWnd = ::vkUI::Engine::getDrawingWindow();
        // for(size_t i=0; i<size; i++){ hWnd->AddPoint(cloud[i].pos, cloud[i].col, display_size); }
        for(size_t i=0; i<size; i++){ 
            hWnd->AddPoint({cloud[i].pos[0], cloud[i].pos[1], cloud[i].pos[2]}, cloud[i].col, display_size);
        }
    }
#else
	void render()override{
        const auto hWnd = ::vkUI::Engine::getDrawingWindow();
        auto *vertex_ptr = hWnd->getVertexPtr();
        const size_t start_idx = vertex_ptr->size() ;
        vertex_ptr->resize(start_idx + 3*size+100);
        // #pragma omp parallel for
        for(size_t i=0; i<size; i++){ 
            (*vertex_ptr)[start_idx + i*3  ].pos[0] = cloud[i].pos[0];
            (*vertex_ptr)[start_idx + i*3  ].pos[1] = cloud[i].pos[1];
            (*vertex_ptr)[start_idx + i*3  ].pos[2] = cloud[i].pos[2]+display_size;

            (*vertex_ptr)[start_idx + i*3+1].pos[0] = cloud[i].pos[0];
            (*vertex_ptr)[start_idx + i*3+1].pos[1] = cloud[i].pos[1]+display_size;
            (*vertex_ptr)[start_idx + i*3+1].pos[2] = cloud[i].pos[2];

            (*vertex_ptr)[start_idx + i*3+2].pos[0] = cloud[i].pos[0]+display_size;
            (*vertex_ptr)[start_idx + i*3+2].pos[1] = cloud[i].pos[1];
            (*vertex_ptr)[start_idx + i*3+2].pos[2] = cloud[i].pos[2];

            (*vertex_ptr)[start_idx + i*3  ].col[0] = cloud[i].col[0]; (*vertex_ptr)[start_idx + i*3  ].col[1] = cloud[i].col[1]; (*vertex_ptr)[start_idx + i*3  ].col[2] = cloud[i].col[2];
            (*vertex_ptr)[start_idx + i*3+1].col[0] = cloud[i].col[0]; (*vertex_ptr)[start_idx + i*3+1].col[1] = cloud[i].col[1]; (*vertex_ptr)[start_idx + i*3+1].col[2] = cloud[i].col[2];
            (*vertex_ptr)[start_idx + i*3+2].col[0] = cloud[i].col[0]; (*vertex_ptr)[start_idx + i*3+2].col[1] = cloud[i].col[1]; (*vertex_ptr)[start_idx + i*3+2].col[2] = cloud[i].col[2];
            // (*vertex_ptr)[start_idx + i*3  ] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(0.0f, 0.0f, display_size), col);
            // (*vertex_ptr)[start_idx + i*3+1] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(display_size, 0.0f, 0.0f), col);
            // (*vertex_ptr)[start_idx + i*3+2] = ::vkUI::Engine::Vertex(cloud[i] + Vector3(0.0f, display_size, 0.0f), col);
        }
    }
#endif
};

#endif

class uiCoordinate : public uiWidget{
private:
    Vector3 pos;
    double size;
    float line_width;
public:
    uiCoordinate(float _size = 10){ size = _size; pos = {0, 0, 0}; line_width = 1; }
    uiCoordinate(Vector3 _pos, float _size){ size = _size; pos = _pos; size=_size; line_width = size/50;}
    uiCoordinate(Vector3 _pos, float _size, float _line_width){ size = _size; pos = _pos; line_width = _line_width;}
    void setSize(float _size){size = _size;}
	bool CallbackFunc([[maybe_unused]]uiCallbackFlags flag, [[maybe_unused]]Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings) override{ return true; };
	void render()override{
        const auto hWnd = ::vkUI::Engine::getDrawingWindow();
        hWnd->AddLine(pos, pos + Vector3(size, 0.0f, 0.0f), {255, 10, 10}, line_width);
        hWnd->AddLine(pos, pos + Vector3(0.0f, size, 0.0f), {10, 255, 10}, line_width);
        hWnd->AddLine(pos, pos + Vector3(0.0f, 0.0f, size), {10, 10, 255}, line_width);
    }
};

class uiCylinder : public uiWidget{
private:
    Vector3 p, n;
    double r;
    Vector3b col;
    /* float line_width; */
    WidgetDrawTypes drawtype;
public:
    uiCylinder(Vector3 pos, Vector3 normal, double _r, Vector3b _col = {255, 255, 255}){ p = pos; n =normal; r = _r; col=_col; drawtype = WidgetDrawTypes::DrawAsSurface; } 
    void setColor(const Vector3b _col){ col = _col; }
    void setDrawType(const WidgetDrawTypes & t){drawtype = t;}
	bool CallbackFunc([[maybe_unused]]uiCallbackFlags flag, [[maybe_unused]]Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings) override{ return true; };
	void render()override{
        const int N= 20; // 分割数
        auto dp = n.normalize();
        Vector3 e1, e2;
        if( dp[0] >= dp[1] && dp[0] >= dp[2]){ // X軸に近い直線
            e1 = {dp[1], dp[0], dp[2]}; e2 = {dp[2], dp[1], dp[0]};
        }else if(dp[1] >= dp[0] && dp[1] >= dp[2]){
            e1 = {dp[1], dp[0], dp[2]}; e2 = {dp[0], dp[2], dp[1]};
        }else{
            e1 = {dp[2], dp[1], dp[0]}; e2 = {dp[0], dp[2], dp[1]};
        }

        e1 = dp.cross(e1); e1 = e1.normalize();
        e2 = dp.cross(e1); e2 = e2.normalize();
        const auto hWnd = ::vkUI::Engine::getDrawingWindow();
        switch(drawtype){
        case WidgetDrawTypes::DrawAsSurface:
            for(int i=0; i<N+1; i++){
                const double theta1 = 6.28 *(float)i     / N;
                const double theta2 = 6.28 *(float)(i+1) / N;
                const auto v1 = (e1*std::cos(theta1) + e2*std::sin(theta1)) * r;
                const auto v2 = (e1*std::cos(theta2) + e2*std::sin(theta2)) * r;
                hWnd->AddQuad( p-n/2 + v1, p-n/2 + v2, p+n/2 + v2, p+n/2 + v1, col );
                hWnd->AddTriangle(p - n/2, p-n/2 + v2, p-n/2+v1, col);
                hWnd->AddTriangle(p + n/2, p+n/2 + v1, p+n/2+v2, col);
            }
            break;

        case WidgetDrawTypes::DrawAsLines:
            for(int i=0; i<N+1; i++){
                const double theta1 = 6.28 *(float)i     / N;
                const double theta2 = 6.28 *(float)(i+1) / N;
                const auto v1 = (e1*std::cos(theta1) + e2*std::sin(theta1)) * r;
                const auto v2 = (e1*std::cos(theta2) + e2*std::sin(theta2)) * r;
                hWnd->AddQuad( p-n/2 + v1, p-n/2 + v2, p+n/2 + v2, p+n/2 + v1, col, r/50);
            }
            break;

        default:
            break;
        }
    }
};

class uiSphere : public uiWidget{
private:
    float size;
    Vector3b col;
    Vector3 pos;
    WidgetDrawTypes drawtype;
public:
    uiSphere(Vector3 _pos, float _size, Vector3b _col = {255, 255, 255}){ size = _size; pos = _pos; col = _col; drawtype = WidgetDrawTypes::DrawAsSurface; } 
    void setDrawType(const WidgetDrawTypes & t){drawtype = t;}
    void setColor(const Vector3b _col){ col = _col; }
	bool CallbackFunc([[maybe_unused]]uiCallbackFlags flag, [[maybe_unused]]Vector2d vec2_1, [[maybe_unused]]int num_1, [[maybe_unused]]int num_2, [[maybe_unused]]const char **strings) override{ return true; };
	void render()override{
        const auto hWnd = ::vkUI::Engine::getDrawingWindow();
        const double t = (1+sqrt(5)) / 2;
        const Vector3 points[] = {
            pos + Vector3(0.0f, -1.0f, t) * size, 
            pos + Vector3(0.0f, 1.0f, t) * size, 
            
            pos + Vector3(t, 0.0f, 1.0f) * size, 
            pos + Vector3(-t, 0.0f, 1.0f) * size, 
            
            pos + Vector3(1.0f, -t, 0.0f) * size, 
            pos + Vector3(1.0f, t, 0.0f) * size, 
            pos + Vector3(-1.0f, t, 0.0f) * size, 
            pos + Vector3(-1.0f, -t, 0.0f) * size, 
            
            pos + Vector3(t, 0.0f, -1.0f) * size, 
            pos + Vector3(-t, 0.0f, -1.0f) * size, 
            
            pos + Vector3(0.0f, -1.0f, -t) * size, 
            pos + Vector3(0.0f, 1.0f, -t) * size, 
        };
        float width = -1;
        if(drawtype == WidgetDrawTypes::DrawAsLines){ width = 2.0f; }

        hWnd->AddTriangle(points[0], points[2], points[1], col, width);
        hWnd->AddTriangle(points[1], points[2], points[5], col, width);
        hWnd->AddTriangle(points[1], points[5], points[6], col, width);
        hWnd->AddTriangle(points[1], points[6], points[3], col, width);
        hWnd->AddTriangle(points[1], points[3], points[0], col, width);
        hWnd->AddTriangle(points[0], points[3], points[7], col, width);
        hWnd->AddTriangle(points[0], points[7], points[4], col, width);
        hWnd->AddTriangle(points[0], points[4], points[2], col, width);

        hWnd->AddTriangle(points[11], points[8], points[10], col, width);
        hWnd->AddTriangle(points[11], points[5], points[8], col, width);
        hWnd->AddTriangle(points[11], points[6], points[5], col, width);
        hWnd->AddTriangle(points[11], points[9], points[6], col, width);
        hWnd->AddTriangle(points[11], points[10], points[9], col, width);
        hWnd->AddTriangle(points[9], points[10], points[7], col, width);
        hWnd->AddTriangle(points[10], points[4], points[7], col, width);
        hWnd->AddTriangle(points[10], points[8], points[4], col, width);
        
        hWnd->AddTriangle(points[5], points[2], points[8], col, width);
        hWnd->AddTriangle(points[3], points[6], points[9], col, width);
        hWnd->AddTriangle(points[7], points[3], points[9], col, width);
        hWnd->AddTriangle(points[2], points[4], points[8], col, width);
    }
};

} // namespace
