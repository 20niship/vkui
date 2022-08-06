#include <engine.hpp>
#include <thread>

int main() {
  try {
    vkUI::init();
    auto wnd = vkUI::addWindow("test", 640, 480);
    vkUI::initFinish();
    bool loop = true;
    while(loop) {
      wnd->updateVertexBuffer();
      const auto fw = vkUI::getTextRendererPtr()->TexWidth;
      const auto fh = vkUI::getTextRendererPtr()->TexHeight;
      wnd->drawDevelopperHelps();
      loop = vkUI::render();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    vkUI::Terminate(true);
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
