#pragma once
#include <cutil/vector.hpp>
#include <drawdata.hpp>
#include <gl/internal.hpp>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


namespace vkUI::Render {

class uiShader {
private:
  const char* vertex_shader;
  const char* fragment_shader;
  GLuint vs, fs;
  GLuint shader_id;

public:
  uiShader();
  ~uiShader();
  void EnableShader();
  void setup();
  inline GLuint getShaderID() { return shader_id; }
};



class uiTexture {
private:
  uiVector<GLuint> texIDs; // ID。
  GLuint TexID_chars;      //文字表示用のテクスチャ
  GLuint TexID_icons;      //アイコン表示用のテクスチャ

public:
  // テクスチャ作成する
  // format = GL_RGB : カラー画像、GL_COLOR_INDEX：単一の値で構成されるカラー指標
  // filter_type = GL_NEAREST, GL_LINEARがある
  GLuint loadTexture(GLubyte* tex_data, int w, int h, GLenum format = GL_RGB, GLint filter = GL_NEAREST, GLuint type=GL_BITMAP);
  GLuint loadTextureFromFile(std::string filename);
  void deleteTexture(GLuint texture) {
    glDeleteTextures(1, &texture);
    texIDs.find_erase(texture);
  }
  void deleteAllTextures();
  inline GLuint getTexID_chars() { return TexID_chars; }
  inline GLuint getTexID_icons() { return TexID_icons; }
  inline void setTexID_chars(GLuint index) {
    TexID_chars = index;
    texIDs.push_back(index);
  }
};


class glWndRender {
private:
  GLuint vao;
  GLFWwindow* wnd;
  GLuint vbo, vbo_ui;
  void cleanupSwapChain();
  void recreateSwapChain();
  void beforeRender();

public:
  glWndRender() = default;
  ~glWndRender() = default;
  void clearVBOs();
  void Render();

  /* inline int getCmdNum() { return vertex_array.size(); } */

  void init();
  void update_wndsize() {}
  void draw(::vkUI::Engine::glDrawData* dd);
  void terminate();
  void createSurface(GLFWwindow* window);
};

class glRender {
private:
  uiTexture texture_renderer;
  uiShader shader;
  bool isSetupFinished;

public:
  void init();
  void terminate();
  auto get_shader() { return &shader; }
  auto get_texuture_renderer() { return &texture_renderer; }
};

} // namespace vkUI::Render
