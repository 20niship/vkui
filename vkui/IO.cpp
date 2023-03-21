#include <IO.hpp>
#include <cutil/vector.hpp>

namespace vkUI::IO {


void vkIO::setup() {
  glfwSetWindowUserPointer(glfw_wnd, this);
  glfwSetFramebufferSizeCallback(glfw_wnd, resizeCB_static);
  glfwSetCursorPosCallback(glfw_wnd, mouseCB_static);
  glfwSetKeyCallback(glfw_wnd, keyboardCB_static);
  glfwSetScrollCallback(glfw_wnd, scrollCB_static);
  glfwSetCharCallback(glfw_wnd, charCB_static);
  glfwSetMouseButtonCallback(glfw_wnd, mouseButtonCB_static);
  /* glfwSetDropCallback(glfw_wnd, drop_callback); */
}


void vkIO::resizeCB(int w, int h) {
  resized_dirty = true;
  wnd_size = {w, h};
}

void mouseCB(double x, double y);

void vkIO::charCB(unsigned int) {}

void vkIO::keyboardCB(int, int, int, int) {
  // if (key == GLFW_KEY_W && action == GLFW_PRESS){ camera_position.pos[2]+= 0.1; }
  // if (key == GLFW_KEY_S && action == GLFW_PRESS){ camera_position.pos[2]-= 0.1; }
  // if (key == GLFW_KEY_A && action == GLFW_PRESS){ camera_position.pos[0]+= 0.1; }
  // if (key == GLFW_KEY_D && action == GLFW_PRESS){ camera_position.pos[0]-= 0.1; }
  // if (key == GLFW_KEY_Q && action == GLFW_PRESS){ camera_position.pos[1]+= 0.1; }
  // if (key == GLFW_KEY_E && action == GLFW_PRESS){ camera_position.pos[1]-= 0.1; }
  double xpos, ypos;
  glfwGetCursorPos(glfw_wnd, &xpos, &ypos);
  mouse_pos = {(int)xpos, (int)ypos};
}

void vkIO::scrollCB(double xoffset, double yoffset) {
  mouse_wheel = {(int)xoffset, (int)yoffset};
}

void vkIO::mouseBtnCB(int button, int action, [[maybe_unused]] int mods) {
  double xpos, ypos;
  glfwGetCursorPos(glfw_wnd, &xpos, &ypos);
  wnd_pos = {(int)xpos, (int)ypos};

  if(action == GLFW_RELEASE) {
    switch(button) {
      case GLFW_MOUSE_BUTTON_RIGHT: break;
      case GLFW_MOUSE_BUTTON_LEFT: break;
      case GLFW_MOUSE_BUTTON_MIDDLE: break;
      default: MY_ASSERT("undefinded mouse button!\n");
    }
  } else {
    switch(button) {
      case GLFW_MOUSE_BUTTON_RIGHT: break;
      case GLFW_MOUSE_BUTTON_LEFT: break;
      case GLFW_MOUSE_BUTTON_MIDDLE: break;
      default: MY_ASSERT("undefinded mouse button!\n");
    }
  }
}

#if 0
void vkIO::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(key == GLFW_KEY_E && action == GLFW_PRESS) activate_airship();
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {}


static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) popup_menu();
}
void drop_callback(GLFWwindow* window, int count, const char** paths) {
  int i;
  for(i = 0; i < count; i++) handle_dropped_file(paths[i]);
}

#endif
} // namespace vkUI::IO
