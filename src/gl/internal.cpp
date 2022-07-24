#pragma once
#include <engine.hpp>
#include <gl/internal.hpp>
#include <iostream>


namespace vkUI::Render {

auto getShaderObject(){ return Engine::getContextPtr()->renderer.get_shader(); }
auto getTextureRenderer(){ return Engine::getContextPtr()->renderer.get_texuture_renderer(); }

void glWndRender::beforeRender() {
}

void glWndRender::draw(::vkUI::Engine::glDrawData* dd) {
  beforeRender();
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, dd->vertex_array.size_in_bytes(), (const GLvoid*)dd->vertex_array.Data, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
  glBufferData(GL_ARRAY_BUFFER, dd->col_array.size_in_bytes(), (const GLvoid*)dd->col_array.Data, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
  glBufferData(GL_ARRAY_BUFFER, dd->cord_array.size_in_bytes(), (const GLvoid*)dd->cord_array.Data, GL_STATIC_DRAW);

  int window_width, window_height;
  assert(wnd != nullptr);
  assert(wnd != NULL);
  glfwGetWindowSize(wnd, &window_width, &window_height);

  const float projectionMatrix[4][4] = {{2.0f / float(window_width), 0.0f, 0.0f, 0.0f}, {0.0f, -2.0f / float(window_height), 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f, 1.0f}};

  auto shader = getShaderObject();
  auto projMatLoc = glGetUniformLocation(shader->getShaderID(), "projectionMatrix");
  auto textureLoc = glGetUniformLocation(shader->getShaderID(), "texture");
  auto uvSizeLoc = glGetUniformLocation(shader->getShaderID(), "uvSize");

  shader->EnableShader();

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  auto textrenderer = ::vkUI::Engine::getTextRendererPtr();
  glUniform2f(uvSizeLoc, 1.0f / float(textrenderer->getTexWidth()), 1.0f / float(textrenderer->getTexHeight()));

  // disp(1.0f / float(textrenderer.getTexWidth()));
  // disp(1.0f/float(textrenderer.getTexWidth()));
  // disp(vertex_array.size());

  glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
  glUniform1i(textureLoc, 0);

  glLineWidth(1.0f);
  glViewport(0, 0, window_width, window_height);


  // glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
  auto texture_obj = getTextureRenderer();
  glBindTexture(GL_TEXTURE_2D, texture_obj->getTexID_chars());
  glDrawArrays(GL_QUADS, 0, dd->vertex_array.size() / 2);

  // for (int order_index = 0; order_index < draw_orders.size()-1; order_index += 3) {
  // 	if (draw_orders[order_index] == CMD_LIST_DRAW_TEXTURE2D) {
  // 		glBindTexture(GL_TEXTURE_2D, draw_orders[order_index + 2]);
  // 		glDrawArrays(GL_TRIANGLE_STRIP, draw_orders[order_index + 1], 4);
  // 	}
  // 	else {
  // 		glBindTexture(GL_TEXTURE_2D, textureObj.whiteTexture);
  // 		glDrawArrays(draw_orders[order_index], draw_orders[order_index + 1], draw_orders[order_index + 2]);
  // 	}
  // }

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}


void glWndRender::createSurface(GLFWwindow* glfw_window) {
  setWindow(glfw_window);
  glfwMakeContextCurrent(glfw_window);

  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  const GLubyte* renderer;
  const GLubyte* version;
  renderer = glGetString(GL_RENDERER);
  version = glGetString(GL_VERSION);
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  glDisable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void glWndRender::terminate() {
  glDeleteBuffers(1, &vertex_vbo);
  glDeleteBuffers(1, &color_vbo);
  glDeleteBuffers(1, &uv_vbo);
  glDeleteVertexArrays(1, &vao);
}


void glWndRender::init() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vertex_vbo);
  glGenBuffers(1, &color_vbo);
  glGenBuffers(1, &uv_vbo);
}


void glRender::init() {
  auto shader = getShaderObject();
  auto texture = getTextureRenderer();

  static bool isSetupFinished = false;

  if(!isSetupFinished) {
    auto fontobj = ::vkUI::Engine::getTextRendererPtr();
    auto idtemp = texture->loadTexture(fontobj->getData(), fontobj->getTexWidth(), fontobj->getTexHeight(), GL_COLOR_INDEX, GL_NEAREST);
    texture->setTexID_chars(idtemp);

    // shaderのセットアップ
    shader->setup();

    isSetupFinished = true;

    //テキストレンダラーのセットアップ
    // switched to glUI::startApp() function in glUI.cpp
    // auto font = ::vkUI::Engine::getTextRenderer();
    // font->setLanguage(FontLanguage::Japansese);
    // font->build();


    //テクスチャのセットアップ
    // switched to glUI::startApp() function in glUI.cpp
    // https://stackoverflow.com/questions/327642/opengl-and-monochrome-texture
    // auto idtemp = texture->loadTexture(font->getData(), font->getTexWidth(), font->getTexHeight(), GL_COLOR_INDEX, GL_NEAREST);
    // texture->setTexID_chars(idtemp);
  }
}

void glRender::terminate() {
  std::cout << "BBB\n";
  auto shader = getShaderObject();
  std::cout << "CCC\n";
  shader->~uiShader();
  std::cout << "DDD\n";
  auto texture = getTextureRenderer();
  std::cout << "EEE\n";
  texture->deleteAllTextures();
  std::cout << "FFF\n";
  glfwTerminate();
  std::cout << "GGGG\n";
  auto textRenderer = ::vkUI::Engine::getTextRendererPtr();
  std::cout << "FFF\n";
  textRenderer->~uiFont();
  std::cout << "done!!\n";
}



// テクスチャ作成する
// format = GL_RGB : カラー画像、GL_COLOR_INDEX：単一の値で構成されるカラー指標
// filter_type = GL_NEAREST, GL_LINEARがある
GLuint uiTexture::loadTexture(GLubyte* tex_data, int w, int h, GLenum format, GLint filter) {
  GLuint idTemp;
  std::cout << "loading texture... (" << w << ", " << h << ")";
  glGenTextures(1, &idTemp);

  float index[] = {0.0, 1.0};

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 2, index);
  glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 2, index);
  glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 2, index);
  glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, index);

  // テクスチャをGPUに転送
  glBindTexture(GL_TEXTURE_2D, idTemp);
  // void glTexImage2D(GLenum target,
  // 　				GLint level,
  // 　				GLint internalFormat,
  // 　				GLsizei width,
  // 　				GLsizei height,
  // 　				GLint border,
  // 　				GLenum format,
  // 　				GLenum type,
  // 　				const void * data);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, GL_BITMAP, tex_data);

  // テクスチャを拡大縮小する時のフィルタリング方法を指定
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  //ラッピング方法を指定
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // テクスチャのアンバインド
  glBindTexture(GL_TEXTURE_2D, 0);

  std::cout << "Done!\n";

  return idTemp;
}


#if 0
GLuint uiTexture::loadTextureFromFile(std::string filename) {
  // Raw画像の場合
  // // ファイルの読み込み
  // std::ifstream fstr(filename, std::ios::binary);
  // const size_t fileSize = static_cast<size_t>(fstr.seekg(0, fstr.end).tellg());
  // fstr.seekg(0, fstr.beg);
  // char* textureBuffer = new char[fileSize];
  // fstr.read(textureBuffer, fileSize);

  cv::Mat img = cv::imread(filename);
  if(img.empty()) return 0;
  GLuint idtemp = loadTexture(img.data, img.rows, img.cols, GL_RGB, GL_LINEAR);
  texIDs.push_back(idtemp);
  return idtemp;
}

#endif


void uiTexture::deleteAllTextures() {
  for(int i = 0; i < texIDs.size(); i++) {
    glDeleteTextures(1, &texIDs[i]);
  }
  texIDs.clear();
}
uiShader::uiShader() {}
void uiShader::EnableShader() {
  glUseProgram(shader_id);
}
void uiShader::setup() {
  std::cout << "shader setup...\n";
  vertex_shader = "#version 400\n"
                  "layout(location = 0) in vec2 position;\n"
                  "layout(location = 1) in vec2 vuv;\n"
                  "layout(location = 2) in vec3 color;\n"
                  "uniform mat4 projectionMatrix;"
                  "uniform vec2 uvSize;"
                  "out vec2 Flag_uv;"
                  "out vec3 outColor;"
                  "void main(void) {"
                  "outColor = color;"
                  "Flag_uv = vec2(vuv.x*uvSize.x, vuv.y*uvSize.y);"
                  //"Flag_uv = vuv;n"
                  "gl_Position = projectionMatrix *  vec4(position, 0.0f, 1.0f);"
                  "}\n";

  fragment_shader = "#version 400\n"
                    "in vec2 Flag_uv;"
                    "in vec3 outColor;"
                    "uniform sampler2D texture;"
                    "out vec4 outFragmentColor; \n"
                    "void main(void) {"
                    "outFragmentColor = vec4(outColor, 1.0) * texture2D(texture, Flag_uv);"
                    //"outFragmentColor = vec4(outColor, 1.0) * texture2D(texture, Flag_uv); n"
                    "}\n";

  vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);

  fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);

  shader_id = glCreateProgram();


  GLint success = 0;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if(success == GL_FALSE) {
    std::cerr << "[ERROR] Vertex shader Compile Failed\n";
  } else {
    std::cout << "vertex Shader compile success!!\n";
  }

  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if(success == GL_FALSE) {
    std::cerr << "フラグメントシェーダー作成に失敗！！";
  } else {
    std::cout << "fragment Shader compile success!!\n";
  }
  glAttachShader(shader_id, vs);
  glAttachShader(shader_id, fs);
  glLinkProgram(shader_id);
}

uiShader::~uiShader() {
  glDeleteShader(shader_id);
}

} // namespace vkUI::Render
