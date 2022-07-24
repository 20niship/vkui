#include <engine.hpp>
#include <thread>

int main() {
  try {
    vkUI::Engine::init();
    auto wnd = vkUI::Engine::addWindow("test", 640, 480);
    vkUI::Engine::initFinish();
    bool loop = true;
    while(loop) {
      wnd->updateVertexBuffer();
      const auto fw = vkUI::Engine::getTextRendererPtr()->TexWidth;
      const auto fh = vkUI::Engine::getTextRendererPtr()->TexHeight;
      wnd->drawDevelopperHelps();
      loop = vkUI::Engine::render();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    vkUI::Engine::Terminate(true);
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
