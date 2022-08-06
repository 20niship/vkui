#include <engine.hpp>
#include <widget.hpp>
#include <widgets/basic_2d.hpp>
#include <widgets/basic.hpp>

using namespace Cutil;

Vector3 pos = {2, 3, 4}; //{1, 2, 3};
Vector3 dir = {5, 5, 5};
Vector3 u = {0, 1, 0};
double scale = 1;


using namespace vkUI;

const int nCloud = 128000;
Vector3 points[nCloud];
#include <fstream>
#include <random>


void make_cloud(){
  // メルセンヌ・ツイスター法による擬似乱数生成器を、
  // ハードウェア乱数をシードにして初期化
  std::random_device seed_gen;
  std::mt19937 engine(seed_gen());
  std::uniform_real_distribution<> dist1(10, 600);
  for (size_t i = 0; i < nCloud; i++) {
        double x = dist1(engine);
        double y = dist1(engine);
        double z = dist1(engine);
        points[i] = {x, y, z};
  }
}

int main() {
    make_cloud();
    try {
        vkUI::init();
        auto wnd = vkUI::addWindow("test", 640,480);
        auto cylinder = vkUI::uiCylinder({0, 0, 0}, {0, 0, 100}, 3, {255, 0, 0});
        auto coord = vkUI::uiCoordinate({0, 0, 0}, 500);
        auto coord2 = vkUI::uiCoordinate({10, 10, 10}, 15);
        cylinder.setDrawType(WidgetDrawTypes::DrawAsLines);
        wnd->addWidget(&coord);
        /* wnd->addWidget(&cylinder); */
        /* wnd->addWidget(&coord2); */
        wnd->setCameraPos({60, 60, 60});
        vkUI::initFinish();
        bool loop = true;
        wnd->setCameraScale(0.03);
        wnd->setCameraTarget({10, 10, 10});
        wnd->setCameraZclip(1, 2000);
        wnd->setCameraPos({1200,1200,600});
        while(loop){
            wnd->updateVertexBuffer();
            wnd->AddString2D("123-aa", {50, 100}, 1);
            loop = vkUI::render();
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
