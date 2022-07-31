#include <engine.hpp>
#include <gl/internal.hpp>
#include <iostream>
#include <logger.hpp>

namespace vkUI::Render {

auto getShaderObject() {
  return Engine::getContextPtr()->renderer.get_shader();
}
auto getTextureRenderer() {
  return Engine::getContextPtr()->renderer.get_texuture_renderer();
}


void glWndRender::draw(::vkUI::Engine::glDrawData* dd) {
  assert(dd->vertex_array.size() > 0);
  assert(dd->col_array.size() > 0);
  assert(dd->cord_array.size() > 0);
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

  assert(window_width > 0);
  assert(window_height > 0);
  const float projectionMatrix[4][4] = {{2.0f / float(window_width), 0.0f, 0.0f, 0.0f}, {0.0f, -2.0f / float(window_height), 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f, 1.0f}};

  auto shader = getShaderObject();
  const auto projMatLoc = glGetUniformLocation(shader->getShaderID(), "projectionMatrix");
  const auto textureLoc = glGetUniformLocation(shader->getShaderID(), "texture");
  const auto uvSizeLoc = glGetUniformLocation(shader->getShaderID(), "uvsize");

#if 0
  const GLuint vertLoc = 0;
  const GLuint uvLog = 1;
  const GLuint colLoc = 2;
#else
  const auto vertLoc = (GLuint)glGetAttribLocation(shader->getShaderID(), "position");
  const auto uvLoc = (GLuint)glGetAttribLocation(shader->getShaderID(), "vuv");
  const auto colLoc = (GLuint)glGetAttribLocation(shader->getShaderID(), "color");
#endif
  /* assert(vertLoc >= 0); */
  /* assert(uvSizeLoc >= 0); */
  /* assert(uvLoc >= 0); */
  /* assert(colLoc >= 0); */
  /* assert(projMatLoc >= 0); */
  /* assert(textureLoc >= 0); */

  shader->EnableShader();

  /* glEnableVertexAttribArray(0); */
  /* glEnableVertexAttribArray(1); */
  /* glEnableVertexAttribArray(2); */


  glEnableVertexAttribArray(vertLoc);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glVertexAttribPointer(vertLoc, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, (GLvoid*)0);

  glEnableVertexAttribArray(colLoc);
  glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
  glVertexAttribPointer(colLoc, 3, GL_UNSIGNED_BYTE, GL_FALSE, 0, (GLvoid*)0);

  glEnableVertexAttribArray(uvLoc);
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
  glVertexAttribPointer(uvLoc, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, (GLvoid*)0);

  auto textrenderer = ::vkUI::Engine::getTextRendererPtr();
  glUniform2f(uvSizeLoc, 1.0f / float(textrenderer->getTexWidth()), 1.0f / float(textrenderer->getTexHeight()));
  disp(textrenderer->getTexWidth());
  disp(textrenderer->getTexHeight());

  // disp(1.0f / float(textrenderer.getTexWidth()));
  // disp(1.0f/float(textrenderer.getTexWidth()));
  // disp(vertex_array.size());

  glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
  glUniform1i(textureLoc, 0);

  glLineWidth(1.0f);
  glPointSize(10);
  glViewport(0, 0, window_width, window_height);

  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0, 0.0, 0.0, 1.0);

  // glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
  auto texture_obj = getTextureRenderer();
  glBindTexture(GL_TEXTURE_2D, texture_obj->getTexID_chars());
  glDrawArrays(GL_TRIANGLES, 0, dd->vertex_array.size() / 3);

  disp(dd->vertex_array.size());

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
  assert(glfw_window != nullptr);
  wnd = glfw_window;
  glfwMakeContextCurrent(glfw_window);
  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  if(glewInit() != GLEW_OK) {
    uiLOGE << "glewInit() failed!";
    return;
  }

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(
    [](auto source, auto type, auto id, auto severity, auto length, const auto* msg, const void* userProgram) {
      const auto t = type == GL_DEBUG_TYPE_ERROR ? "*** GL DEBUG ERROR ***" : "";
      std::cerr << "GL CALLBACK : " << t << "type = " << type << ", severity = " << severity << ", msg -->>" << std::endl << msg << std::endl << "---- " << std::endl;
      if(type == GL_DEBUG_TYPE_ERROR) {
        throw std::runtime_error("OpenGL Callback ERrror!");
      }
    },
    0);

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
  std::cout << "genbuffer called!" << std::endl;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vertex_vbo);
  glGenBuffers(1, &color_vbo);
  glGenBuffers(1, &uv_vbo);
}


void glRender::init() {
  const auto engine = ::vkUI::Engine::getContextPtr();
  for(auto&& w : engine->windows) w->get_renderer()->createSurface(w->getGLFWwindow());
  for(auto&& w : engine->windows) w->init();

  auto shader = getShaderObject();
  auto texture = getTextureRenderer();

  auto fontobj = ::vkUI::Engine::getTextRendererPtr();
  auto idtemp = texture->loadTexture(fontobj->getData(), fontobj->getTexWidth(), fontobj->getTexHeight(), GL_LUMINANCE, GL_NEAREST, GL_UNSIGNED_BYTE);
  texture->setTexID_chars(idtemp);

  // shaderのセットアップ
  shader->setup();
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

void glRender::terminate() {
  auto shader = getShaderObject();
  shader->~uiShader();
  auto texture = getTextureRenderer();
  texture->deleteAllTextures();
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
GLuint uiTexture::loadTexture(GLubyte* tex_data, int w, int h, GLenum format, GLint filter, GLuint type) {
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, format, type, tex_data);

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
#if 0
  vertex_shader = /* "#version 400\n" */
    "#version 130\n"
    "layout(location = 0) in vec2 position;\n"
    "layout(location = 1) in vec2 vuv;\n"
    "layout(location = 2) in vec3 color;\n"
    "uniform mat4 projectionMatrix;"
    "uniform vec2 uvsize;"
    "out vec2 Frag_uv;"
    "out vec3 outColor;"
    "void main(void) {"
    "outColor = color;"
    "Frag_uv= vec2(vuv.x*uvsize.x, vuv.y*uvsize.y);"
    //"Frag_uv= vuv;n"
    "gl_Position = projectionMatrix *  vec4(position, 0.0f, 1.0f);"
    "}\n";
#else
  vertex_shader = /* */
    "#version 130\n"
    "uniform vec2 uvsize;\n"
    "in vec2 position;\n"
    "in vec2 vuv;\n"
    "in vec3 color;\n"
    "uniform mat4 projectionMatrix;\n"
    "out vec2 Frag_uv;\n"
    "out vec3 outColor;\n"
    "void main(){"
    "    Frag_uv= vec2(float(vuv.x)*uvsize.x, float(vuv.y)*uvsize.y);"
    "    outColor= vec3(color) / 256.0;\n"
    "    gl_Position = projectionMatrix* vec4(vec2(position.xy),0.0 ,1.0);\n"
    "}\n";
#endif
  fragment_shader = /* "#version 400\n" */
    "#version 130\n"
    "in vec2 Frag_uv;"
    "in vec3 outColor;"
    "uniform sampler2D texture;"
    "out vec4 col; \n"
    "void main(void) {"
    "  col= vec4(outColor, 1.0);"
    "  if(Frag_uv.x != 0.0 || Frag_uv.y != 0.0){"
    "    col.a = texture2D(texture, Frag_uv).x;"
    "  }else{"
    "    col.a = 1.0;"
    "  }"
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
    uiLOGE << "[ERROR] Vertex shader Compile Failed\n";
    exit(0);
  } else {
    std::cout << "vertex Shader compile success!!\n";
  }

  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if(success == GL_FALSE) {
    uiLOGE << "[ERROR] Fragment shader Compile Failed\n";
    exit(0);
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
