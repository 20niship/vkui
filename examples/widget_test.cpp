#include <engine.hpp>
#include <widgets/basic_2d.hpp>
#include <widgets/plot.hpp>
#include <widgets/basic.hpp>
#include <random>
#include <time.h>
#include <icon.hpp>

using namespace vkUI;
using namespace vkUI::Engine;


int values[100];
Vector2 points[100];

void setV(){
  static int n;
  for(int i=0; i<100; i++){
    values[i] = 53*std::sin(0.2f * i - n*0.005) + 30;
  }
  n++;

  srand((int)time(0));
  for(int i=0; i<100; i++){
    double x = rand() % 90;
    double y = rand() % 90;
    points[i] = {x, y};
  }
}

int main() {
    try {
        vkUI::Engine::init();
        auto wnd = vkUI::Engine::addWindow("test", 640,480);
        auto coord = vkUI::uiCoordinate({0, 0, 0}, 500);
        wnd->addWidget(&coord);
        wnd->setCameraPos({60, 60, 60});
        wnd->setCameraScale(0.03);
        wnd->setCameraTarget({10, 10, 10});
        wnd->setCameraZclip(1, 2000);
        wnd->setCameraPos({1200,1200,600});

        auto frame = vkUI::uiFrame("test");
        frame.setSize({300, 500});
        frame.setPos({10, 10});
        wnd->addWidget2D(&frame);
        bool hoge = false;
        float huga = 123.0f;
        Vector3 vec = {10, 20, 30};
        frame.AddWidget(new vkUI::uiCheckbox("CHECK!", &hoge));
        frame.AddWidget(new vkUI::uiLabel("Label seco\noge\nhogeh\nhafelaineh\noge"));
        auto col = vkUI::uiCollapse("collapse test");
        frame.AddWidget(&col);
        col.AddWidget(new vkUI::uiButton("hoge", &hoge));
        col.AddWidget(new vkUI::uiButton("huaga", &hoge));
        col.AddWidget(new vkUI::uiLabel("Label seco\noge\nhogeh\nhafelaineh\noge"));
        Vector3b a;
        col.AddWidget(new vkUI::uiCol("L", &a));
        col.AddWidget(new vkUI::uiCol2("Color", &a));

        frame.AddWidget({
          vkUI::Widget::Button("main", &hoge),
          vkUI::Widget::Label("mai\naa\n15315afan"),
          vkUI::Widget::Label(""),
          vkUI::Widget::Label("Icon Tests = " + vkUI::Icon::FOLDER),
          vkUI::Widget::Slider("aaa", &huga, Vector2(0, 1000)),
          vkUI::Widget::DragVec3("aaa", &vec, Vector2(0, 1000)),
          vkUI::Widget::Table("aaa\tbbb\tccc\n123\r456\r789\naaa\rvvv\rhhh\naa\rhh\rX")
        });

        auto frame2 = vkUI::uiFrame("plot");
        /* auto frame3 = vkUI::uiTextTexture(); */
        auto plot = vkUI::uiPlot("TITLE");
        frame2.setSize({300, 300});
        frame2.setPos({400, 10});
        frame2.AddWidget(&plot);
        wnd->addWidget2D(&frame2);

        vkUI::Engine::initFinish();
        bool loop = true;
        while(loop){
          wnd->updateVertexBuffer();
          wnd->AddStringBalloon("0,0,0", {0,0,0}, 1.0);
          wnd->AddStringBalloon("1000,0,0", {1000,0,0}, 1.0);
          wnd->AddStringBalloon("0,1000,0", {0,1000,0}, 1.0);

            wnd->drawDevelopperHelps();
            loop = vkUI::Engine::render();
        }
      vkUI::Engine::Terminate(true);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
