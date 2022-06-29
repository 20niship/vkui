#pragma once
#include <cmath>
#include <limits>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <array>
#include <optional>
#include <set>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <ft2build.h>
#include <freetype/ftsynth.h>
#include FT_FREETYPE_H

#include "stb/stb_image.h"
#include <myvector.hpp>
#include "enums.hpp"
#include "widget.hpp"

namespace vkUI{
struct uiRect {
	uiRect() {posX = 0;posY = 0;width = 0;height = 0;}
	uiRect(float posX_, float posY_, float width_, float height_){
		posX = posX_;
		posY = posY_;
		width = width_;
		height = height_;
	}
    template <typename U> uiRect(_Vec<U, 2> pos, _Vec<U, 2> size){
        posX = pos[0];
        posY = pos[1];
		width = size[0];
		height = size[1];
    }

	float posX, posY, width, height;
	inline float right() { return posX + width; }
	inline float bottom() { return posY + height; }
	inline bool isContains(const int x, const int y)const {
		return (x >= posX) && (x <= posX + width) && (y >= posY) && (y <= posY + height);
	}
	template <typename U> inline bool isContains(const _Vec<U, 2> other)const { return isContains(other[0], other[1]); }

	inline bool isNoContains(uiRect outside) {
		return (outside.posX > right()) || (outside.right() < posX) || (outside.posY > bottom()) || (outside.bottom() < posY);
	}

	inline Vector2 getPos() { return {posX, posY}; }
	inline Vector2 getSize() { return {width, height}; }
};

class uiWidget;

}

namespace vkUI::Engine{
class uiFont;
class uiWindow;
using uihWnd = uiWindow*;
using uihWidget = uiWidget*;

// keyboardCBで呼ばれる関数
// keycoard, scancode, action, mods, position
using KeyboardFuncT = std::function<bool(int, int, int, int, Vector2)>; 

#define VK_ENGINE_MAX_FRAMES_IN_FLIGHT 2
#define VK_ENGINE_ENABLE_VALIDATION_LAYERS 
#define VKUI_ENGINE_ENABLE_FPS_CALC
#define VKUI_ENGINE_USE_FLOAT_VERTEX

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

// -----------------------------------------------------
//    [SECTION] Buffer
// -----------------------------------------------------
struct uiBuffer{
	vk::Buffer buf, staging_buf;
	vk::DeviceMemory buf_mem, staging_buf_mem;
	vk::UniqueDevice *device_ptr; 
     vk::MemoryPropertyFlags prop;
	vk::DeviceSize buf_size;
     vk::BufferUsageFlags usage;
	bool allocated;
    bool use_staging_buffer;

	uiBuffer();
	uiBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	~uiBuffer();
    inline void setUseStagingBuf(bool v){use_staging_buffer = v;}
	void create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	void __create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer *buf_p, vk::DeviceMemory *mem_p);
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	static std::vector<char> readFile(const std::string& filename);
    vk::CommandBuffer __beginSingleTimeCommands();
    void __endSingleTimeCommands(vk::CommandBuffer commandBuffer);
    void __copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

	void copyData(void *src, size_t size);
	void cleanup();
};


// -----------------------------------------------------
//    [SECTION] Vertex 
// -----------------------------------------------------
struct Vertex {
#ifdef VKUI_ENGINE_USE_FLOAT_VERTEX
    float pos[3];
    // float uv[2];
#else
    int16_t pos[3];
    // uint16_t uv[2];
#endif 
    uint8_t col[3];

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = {};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
#ifdef VKUI_ENGINE_USE_FLOAT_VERTEX
        attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
#else
        attributeDescriptions[0].format = vk::Format::eR16G16B16Sint;
#endif 
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR8G8B8Uint;
        attributeDescriptions[1].offset = offsetof(Vertex, col);

//         attributeDescriptions[2].binding = 0;
//         attributeDescriptions[2].location = 2;
// #ifdef VKUI_ENGINE_USE_FLOAT_VERTEX
//         attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
// #else
//         attributeDescriptions[2].format = vk::Format::eR16G16Uint;
// #endif 
//         attributeDescriptions[2].offset = offsetof(Vertex, uv);
        return attributeDescriptions;
    }

    Vertex(const Vector3 &_pos, const Vector3b &_col /*, const Vector2 _uv = {0.0f, 0.0f } */){
        pos[0] = _pos[0]; pos[1] = _pos[1]; pos[2] = _pos[2];
        col[0] = _col[0]; col[1] = _col[1];  col[2] = _col[2];
        // uv[0]  = _uv[0];  uv[1]  = _uv[1];
    }
};

struct VertexUI {
    int16_t pos[2];
    uint8_t col[3];
    uint16_t uv[2];

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VertexUI);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR16G16Sint;
        attributeDescriptions[0].offset = offsetof(VertexUI, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR8G8B8Uint;
        attributeDescriptions[1].offset = offsetof(VertexUI, col);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR16G16Uint;
        attributeDescriptions[2].offset = offsetof(VertexUI, uv);
        return attributeDescriptions;
    }
    template<typename T>
    VertexUI(const _Vec<T, 2>& _pos, const Vector3b& _col, const _Vec<T, 2>& _uv){
        pos[0] = _pos[0]; pos[1] = _pos[1]; 
        col[0] = _col[0]; col[1] = _col[1];  col[2] = _col[2];
        uv[0]  = _uv[0];  uv[1]  = _uv[1];
    }
};


// -----------------------------------------------------
//    [SECTION] uiWindow 
// -----------------------------------------------------
class uiWindow {
private:
    struct Cursors{
      Cursors()= default;
      void init(){
        arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        ibeam= glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        crosshar = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        hresize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        vresize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
      }
      GLFWcursor* arrow, *ibeam, *crosshar, *hand, *hresize, *vresize;
    }cursors;

    struct CameraPosition{
        Vector3 pos;
        Vector3 dir;
        Vector3 u;
        double scale;

        double fov, zNear, zFar, aspect;
        // 画角、Z座標のクリッピング、アスペクト比

        CameraPosition(){
            pos = {0, 0, 0};
            dir = {1, 1, 1};
            u = {0, 0, -1};
            scale = 0.06;

            fov = 60.0f * 3.1415 / 180.0;
            zNear = 1.0;
            zFar = 600.0;
            aspect = 1.0;
        }

        void move(double n){ pos += dir * n; }  // 視点は動かさず、カメラ座標だけ移動する
        void lookAt(float* matrix){ lookAt(pos[0], pos[1], pos[2], dir[0], dir[1], dir[2], u[0], u[1], u[2], matrix); }

        void go_closer(double delta){
          const auto start = pos + dir;
          pos = start - dir * (1.0 + 0.1*delta);
          dir = start - pos;
        }

        void lookAt(float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz, float *matrix){
            float l;
            tx = ex - tx;
            ty = ey - ty;
            tz = ez - tz;
            l = sqrtf(tx * tx + ty * ty + tz * tz); // TODO: L = 4のときのエラー処理
            matrix[ 2] = tx / l;
            matrix[ 6] = ty / l;
            matrix[10] = tz / l;

            tx = uy * matrix[10] - uz * matrix[ 6];
            ty = uz * matrix[ 2] - ux * matrix[10];
            tz = ux * matrix[ 6] - uy * matrix[ 2];
            l = sqrtf(tx * tx + ty * ty + tz * tz); 
            matrix[ 0] = tx / l;
            matrix[ 4] = ty / l;
            matrix[ 8] = tz / l;

            matrix[ 1] = matrix[ 6] * matrix[ 8] - matrix[10] * matrix[ 4];
            matrix[ 5] = matrix[10] * matrix[ 0] - matrix[ 2] * matrix[ 8];
            matrix[ 9] = matrix[ 2] * matrix[ 4] - matrix[ 6] * matrix[ 0];

            matrix[12] = -(ex * matrix[ 0] + ey * matrix[ 4] + ez * matrix[ 8]);
            matrix[13] = -(ex * matrix[ 1] + ey * matrix[ 5] + ez * matrix[ 9]);
            matrix[14] = -(ex * matrix[ 2] + ey * matrix[ 6] + ez * matrix[10]);

            matrix[ 3] = matrix[ 7] = matrix[11] = 0.0f;
            
            // for(int i=0; i<3; i++){matrix[i] *= scale; matrix[i+4] *= scale; matrix[i+8] *= scale; }

            matrix[2] = -matrix[2];
            matrix[6] = -matrix[6];
            matrix[10] = -matrix[10];
            matrix[14] = -matrix[14];

            // matrix[8] = -matrix[8];
            // matrix[9] = -matrix[9];
            // matrix[10] = -matrix[10];
            // matrix[11] = -matrix[11];
            matrix[15] = 1.0f; //1.0f / scale; //1.0f;
        }
    
        // 透視投影変換行列を求める
        void perspectiveMatrix(float left, float right, float bottom, float top, float near, float far, float *matrix){
            float dx = right - left; // TODO: エラー処理（dx != 0)
            float dy = top - bottom;
            float dz = far - near;
            assert(dx != 0);
            assert(dy != 0);
            assert(dz != 0);
            matrix[ 0] =  2.0f * near / dx;
            matrix[ 5] =  2.0f * near / dy;
            matrix[ 8] =  (right + left) / dx;
            matrix[ 9] =  (top + bottom) / dy;
            matrix[10] = -(far + near) / dz;
            matrix[11] = -1.0f;
            matrix[14] = -2.0f * far * near / dz;
            matrix[ 1] = matrix[ 2] = matrix[ 3] = matrix[ 4] =
            matrix[ 6] = matrix[ 7] = matrix[12] = matrix[13] = matrix[15] = 0.0f;
        }

        // 平行投影変換行列を求める
        void orthogonalMatrix(float left, float right, float bottom, float top, float near, float far, float *matrix){
            float dx = right - left;
            float dy = top - bottom;
            float dz = far - near;
            assert(dx != 0);
            assert(dy != 0);
            assert(dz != 0);

            matrix[ 0] =  2.0f / dx;
            matrix[ 5] =  2.0f / dy;
            matrix[10] = -2.0f / dz;
            matrix[12] = -(right + left) / dx;
            matrix[13] = -(top + bottom) / dy;
            matrix[14] = -(far + near) / dz;
            matrix[15] =  1.0f;
            matrix[ 1] = matrix[ 2] = matrix[ 3] = matrix[ 4] =
            matrix[ 6] = matrix[ 7] = matrix[ 8] = matrix[ 9] = matrix[11] = 0.0f;
        }

        Mat4x4 lookAtRH(Vector3 const& eye, Vector3 const& center, Vector3 const& up){
            const auto f((center - eye).normalize());
            const auto s((f.cross(up).normalize()));
            const auto u(s.cross(f));
            Mat4x4 result; result.all(1.0);
            result(0, 0) = s[0];
            result(0, 1) = s[1];
            result(0, 2) = s[2];
            result(1, 0) = u[0];
            result(1, 1) = u[1];
            result(1, 2) = u[2];
            result(2, 0) =-f[0];
            result(2, 1) =-f[1];
            result(2, 2) =-f[2];
            result(0, 3) =-(s[0]*eye[0] + s[1]*eye[1] + s[2]*eye[2]);
            result(1, 3) =-(u[0]*eye[0] + u[1]*eye[1] + u[2]*eye[2]);
            result(2, 3) = (f[0]*eye[0] + f[1]*eye[1] + f[2]*eye[2]);
            return result;
        }

        Mat4x4 perspective(double fovy, double aspect, double near, double far){
            assert(std::abs(aspect - std::numeric_limits<double>::epsilon()) > 0 );
            const auto tanHalfFovy = std::tan(fovy / 2.0f);
            Mat4x4 result; result.all(0);
            result(0, 0) = 1.0f / (aspect * tanHalfFovy);
            result(1, 1) = 1.0f / (tanHalfFovy);
            result(2, 2) = far / (far - near);
            result(3, 2) = 1.0f;
            result(2, 3) = -(far * near) / (far - near);
            return result;
        }

        void getCameraProjMatrix(float *matrix){
            _Mat<float, 4, 4> view;
            lookAt(pos[0], pos[1], pos[2], dir[0], dir[1], dir[2], u[0], u[1], u[2], view.value);
            const auto projection = perspective(fov, aspect, zNear, zFar);
            const auto output = projection * view;
            for(int i=0; i<16; i++){matrix[i] = output[i]; }
        }
    }camera_position;

	struct UniformData{
		float proj[16];
		float proj_uv[16];
		float texure_size[2];
	}uniform_data;

    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout() ;
    void createGraphicsPipeline();
    void createGraphicsPipeline_UI();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    void createFramebuffers();
    void createCommandBuffers(bool);

    void updateUniformBuffer();
    void renderUI();

    float fps;
	uint64_t currentFrame;
	std::string name;
  Vector2d pos, size;
    ::vkUI::uiRect clipping_rect{0,0,99999,999999};
    KeyboardFuncT user_key_cb;
public:
	struct _wndStyle{
    unsigned char EnableTitleBar   : 1; //Enable title bar
	  unsigned char EnableChildWindow: 1; // Enable child window and show child windows
	  unsigned char EnablePopups     : 1;  // Enable popup Window
		unsigned char isFullScreen     : 1;
	}wndStyle;

	
    uiVector<Vertex> vertices;
    uiVector<VertexUI> vertices_ui;
    ::vkUI::uiRootWidget root_widget;
    ::vkUI::uiRootWidget2D root_widget_ui;
	GLFWwindow *window;
	bool framebufferResized;
    vk::SurfaceKHR surface;
    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;
    std::vector<vk::Framebuffer> swapChainFramebuffers;
    vk::RenderPass renderPass;
    vk::PipelineLayout pipelineLayout, pipelineLayout_ui;
    vk::UniquePipeline graphicsPipeline, graphicsPipeline_ui;
    std::vector<vk::DescriptorSet> descriptorSet;
    vk::DescriptorSetLayout descriptorSetLayout;
	uiBuffer uniformBuffer, vertexBuffer, vertexBuffer_ui;
    std::vector<vk::CommandBuffer, std::allocator<vk::CommandBuffer>> commandBuffers;

    static void resizeCB_static(GLFWwindow* window, int width, int height) { auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window)); app->resizeCB(width, height); }
    static void mouseCB_static(GLFWwindow* window,  double xpos, double ypos){ auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window)); app->mouseCB(xpos, ypos); }
    static void keyboardCB_static(GLFWwindow* window, int key, int scancode, int action, int mods) { auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window)); app->keyboardCB(key, scancode, action, mods); }
    static void scrollCB_static(GLFWwindow* window, double x, double y) { auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window)); app->scrollCB(x, y); }
    static void charCB_static(GLFWwindow* window, unsigned int codepoint){ auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window)); app->charCB(codepoint); }
    static void mouseButtonCB_static(GLFWwindow* window,  int button, int action, int mods){ auto app = reinterpret_cast<uiWindow*>(glfwGetWindowUserPointer(window)); app->mouseBtnCB(button, action, mods); }
public:
	uiWindow(std::string _name, uint16_t width, uint16_t height);
	~uiWindow();
    void cleanupSwapChain();
    void recreateSwapChain();
    SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device);
    void createSurface();
    void drawFrame(const bool verbose=false);
    void drawDevelopperHelps();
    void updateVertexBuffer();
    inline auto getVertexPtr(){ return &vertices; }
    void init();
    inline vk::SurfaceKHR getSurface() { return surface; }
    inline auto getGLFWwindow() const { return window; }
    inline bool wndShouldClose(){ return glfwWindowShouldClose(window);}
    Vector2d getWindowSize()const { int x, y; glfwGetWindowSize(window, &x, &y); return {x,y}; }
    Vector2d getWindowPos()const {return pos; }
#ifdef VKUI_ENGINE_ENABLE_FPS_CALC
    inline float getFPS(){ return fps;}
#endif

    // cursors
    inline void setHandCursor() const { glfwSetCursor(window, cursors.hand); }
    inline void setArrowCursor() const { glfwSetCursor(window, cursors.arrow); }
    inline void setHResizeCursor() const { glfwSetCursor(window, cursors.hresize); }
    inline void setVResizeCursor() const { glfwSetCursor(window, cursors.vresize); }
    inline void setCrossHairCursor() const { glfwSetCursor(window, cursors.crosshar); }
    inline void setIbeamCursor() const { glfwSetCursor(window, cursors.ibeam); }
    inline void setDefaultCursor() const {glfwSetCursor(window, NULL); }

    inline void setUserKeyboardCB(const KeyboardFuncT &f){ user_key_cb = f; }
    const KeyboardFuncT &getUserKeyboardCB()const{ return user_key_cb; }
    // --------------     rendering functions     -------------
    inline void setClippingRect(const vkUI::uiRect & r){ clipping_rect = r; }
    inline vkUI::uiRect getClippingRect() const { return clipping_rect; }

    inline void resizeCB(int w, int h){
        framebufferResized = true;
        size[0] = w;
        size[1] = h;
        root_widget_ui.needRendering(true);
    }

    void mouseCB(double x, double y);

    void charCB(unsigned int codepoint){
        std::cout << codepoint << std::endl;
        const double scale_ = (camera_position.pos - camera_position.dir).norm();
        if (codepoint == 'w'){ camera_position.dir[2]+= scale_ * 0.01; }
        if (codepoint == 's'){ camera_position.dir[2]-= scale_ * 0.01; }
        if (codepoint == 'a'){ camera_position.dir[0]+= scale_ * 0.01; }
        if (codepoint == 'd'){ camera_position.dir[0]-= scale_ * 0.01; }
        if (codepoint == 'q'){ camera_position.dir[1]+= scale_ * 0.01; }
        if (codepoint == 'e'){ camera_position.dir[1]-= scale_ * 0.01; }
        root_widget_ui.CallbackFunc(uiCallbackFlags::CharInput, {0, 0}, codepoint, 0, nullptr);
    }

    void keyboardCB(int key, int scancode, int action, int mods){
        // if (key == GLFW_KEY_W && action == GLFW_PRESS){ camera_position.pos[2]+= 0.1; }
        // if (key == GLFW_KEY_S && action == GLFW_PRESS){ camera_position.pos[2]-= 0.1; }
        // if (key == GLFW_KEY_A && action == GLFW_PRESS){ camera_position.pos[0]+= 0.1; }
        // if (key == GLFW_KEY_D && action == GLFW_PRESS){ camera_position.pos[0]-= 0.1; }
        // if (key == GLFW_KEY_Q && action == GLFW_PRESS){ camera_position.pos[1]+= 0.1; }
        // if (key == GLFW_KEY_E && action == GLFW_PRESS){ camera_position.pos[1]-= 0.1; }
        root_widget_ui.CallbackFunc(uiCallbackFlags::Keyboard,{ 0, 0} , key, action, nullptr);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        user_key_cb(key, scancode, action, mods, {xpos, ypos});
    }

    void scrollCB(double xoffset, double yoffset);

    void mouseBtnCB(int button, int action, [[maybe_unused]]int mods){ 
     double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos);
		if (action == GLFW_RELEASE){
			switch(button){
				case GLFW_MOUSE_BUTTON_RIGHT:  root_widget_ui.CallbackFunc(uiCallbackFlags::RMouseUP, {(int)xpos, (int)ypos}, 0,0, nullptr);break;
				case GLFW_MOUSE_BUTTON_LEFT :  root_widget_ui.CallbackFunc(uiCallbackFlags::LMouseUP, {(int)xpos, (int)ypos}, 0,0, nullptr);break;
				case GLFW_MOUSE_BUTTON_MIDDLE: root_widget_ui.CallbackFunc(uiCallbackFlags::CMouseUP, {(int)xpos, (int)ypos}, 0,0, nullptr);break;
				default : MY_ASSERT("undefinded mouse button!\n");
			}
		}else{
			switch(button){
				case GLFW_MOUSE_BUTTON_RIGHT:  root_widget_ui.CallbackFunc(uiCallbackFlags::RMouseDown, {(int)xpos, (int)ypos}, 0,0, nullptr);break;
				case GLFW_MOUSE_BUTTON_LEFT :  root_widget_ui.CallbackFunc(uiCallbackFlags::LMouseDown, {(int)xpos, (int)ypos}, 0,0, nullptr);break;
				case GLFW_MOUSE_BUTTON_MIDDLE: root_widget_ui.CallbackFunc(uiCallbackFlags::CMouseDown, {(int)xpos, (int)ypos}, 0,0, nullptr);break;
				default : MY_ASSERT("undefinded mouse button!\n");
				}
		}	
	}

    // ----------  rendering functions --------------
    inline void RemoveVerticies() { vertices.resize(0); }
    inline void __AddPointSizeZero(const Vector3 &pos, const Vector3b &col){ vertices.push_back(std::move(Vertex(pos, col))); }
    [[deprecated]]inline void __AddPointSizeZero(const Vector3 &pos, const Vector3b &col, [[maybe_unused]]const Vector2 &uv ){ vertices.push_back(std::move(Vertex(pos, col))); }
    inline void AddTriangle(const Vector3 &pos1, const Vector3 &pos2, const Vector3 &pos3, const Vector3b &col1, const Vector3b &col2, const Vector3b &col3, const float width = -1){
        if (width < 0){
            __AddPointSizeZero(pos1, col1);  __AddPointSizeZero(pos2, col2); __AddPointSizeZero(pos3, col3); 
        }else {
            AddLine(pos1, pos2, col1, col2, width); AddLine(pos2, pos3, col2, col3, width); AddLine(pos3, pos1, col3, col1, width);
        }
    }
    inline void AddTriangle(const Vector3 &pos1, const Vector3 &pos2, const Vector3 &pos3, const Vector3b &col, const float width = -1) { AddTriangle(pos1, pos2, pos3, col, col, col, width); }
    
    /* inline void AddTriangle(const Vector3 &pos1, const Vector3 &pos2, const Vector3 &pos3, const Vector2 uv1, const Vector2 &uv2, const Vector2 &uv3, const Vector3b &col){ */ 
    /*   __AddPointSizeZero(pos1, col); */
    /*   __AddPointSizeZero(pos2, col); */
    /*   __AddPointSizeZero(pos3, col); */
    /* } */
  
    inline void AddQuad(const Vector3 &pos1, const Vector3 &pos2, const Vector3 &pos3, const Vector3 &pos4, const Vector3b &col, const float width){
        AddLine(pos1, pos2, col, width); 
        AddLine(pos2, pos3, col, width);
        AddLine(pos3, pos4, col, width);
        AddLine(pos4, pos1, col, width);
    }

    inline void AddQuad(const Vector3 &pos1, const Vector3 &pos2, const Vector3 &pos3, const Vector3 &pos4, const Vector3b col){
       AddTriangle(pos1, pos2, pos3, col); 
       AddTriangle(pos1, pos3, pos4, col); 
    }

    inline void AddRect(const Vector3 &pos, const Vector2 &size, const Vector3 &normal, const Vector3b &col){
      const auto hs = size/2;
      const auto dp = normal.normalize();
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
      AddQuad(
          pos +Vector3{-hs[0],  hs[1], 0},
          pos +Vector3{-hs[0], -hs[1], 0},
          pos +Vector3{ hs[0], -hs[1], 0},
          pos +Vector3{ hs[0],  hs[1], 0},
          col
      );
    }

    // draw cyliinder surface
    inline void AddCylinder(const Vector3 &p, const Vector3 n, const double r, const Vector3b &col){
        constexpr int N = 20; // 側面分割数
        const auto dp = n.normalize();
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
        for(int i=0; i<N+1; i++){
            const double theta1 = 6.28 *(float)i     / N;
            const double theta2 = 6.28 *(float)(i+1) / N;
            const auto v1 = (e1*std::cos(theta1) + e2*std::sin(theta1)) * r;
            const auto v2 = (e1*std::cos(theta2) + e2*std::sin(theta2)) * r;
            AddQuad( p-n/2 + v1, p-n/2 + v2, p+n/2 + v2, p+n/2 + v1, col );
            AddTriangle(p - n/2, p-n/2 + v2, p-n/2+v1, col);
            AddTriangle(p + n/2, p+n/2 + v1, p+n/2+v2, col);
        }
    }

    inline void AddCylinderLine(const Vector3 &p, const Vector3 n, const double r, const Vector3b &col, const int width=2){
        constexpr int N = 20; // 側面分割数
        const auto dp = n.normalize();
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
        for(int i=0; i<N+1; i++){
            const double theta1 = 6.28 *(float)i     / N;
            const double theta2 = 6.28 *(float)(i+1) / N;
            const auto v1 = (e1*std::cos(theta1) + e2*std::sin(theta1)) * r;
            const auto v2 = (e1*std::cos(theta2) + e2*std::sin(theta2)) * r;
            AddQuad( p-n/2 + v1, p-n/2 + v2, p+n/2 + v2, p+n/2 + v1, col, width);
        }
    }


    inline void addWidget(::vkUI::uiWidget *w){ root_widget.AddWidget(w); }
    // [[deprecated]] inline void addWidget(::vkUI::uiWidget w){ root_widget.AddWidget(&w); }
    inline uiWidget &addWidget2D(::vkUI::uiWidget *w){ root_widget_ui.AddWidget(w); return root_widget_ui; }
    // [[deprecated]] inline void addWidget2D(::vkUI::uiWidget w){ root_widget_ui.AddWidget(&w); }
    void AddLine(const Vector3 &pos1, const Vector3 &pos2, const Vector3b &col1, const Vector3b &col2, const float width = 1.0);
    void AddLine(const Vector3 &pos1, const Vector3 &pos2, const Vector3b &col, const float width = 1.0);
    void AddPoint(const Vector3 &pos, const Vector3b &col, const double size = 1.0);
    void AddPoint_mono_triangle(const Vector3 &pos, const Vector3b &col, const double size = 1.0);
    void AddSphere_20(const Vector3 &pos, const float size, const Vector3b &col);
    void AddArrow(const Vector3 &pos1, const Vector3 &pos2, const Vector3b &col, float width = 1.0);    
    void AddArrowTo(const Vector3 &pos, const Vector3 &dir, const Vector3b &col, float width);
    void AddCube(const Vector3& pos, const Vector3& whd, const Vector3b& col);
    void AddCubeLineRotated(const Vector3& pos, const Vector3& size, const Vector3& pry, const Vector3b& col, const float width = 1.0);    
    void AddPlane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col);    
    void AddPlane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col, int width);    
    void AddCubeLine(const Vector3& pos, const Vector3& size, const  Vector3b& col, float width = 1.0);
    void AddCubeLine(const Vector3& pos, float size, const Vector3b &col, float width = 1.0);
    void AddCircle(const Vector3& pos, const Vector3 &normal, const Vector3b& col);
    void AddCone(const Vector3& pos, const Vector3 &dir, float size, const Vector3b &col);
    void AddStringBalloon(const std::string &str, const Vector3 &pos, const float size, const Vector3b &col={255,255,255}, const Vector3b &line={255,255,255});
    void AddStringBalloon2D(const std::string &str, const Vector2d &pos, const float size, const Vector3b &col={255,255,255}, const Vector3b &line={255,255,255});
    void AddGridXY(const Vector2 range, const int n=10, const Vector3b col={255,255,0});
    void AddCross(const Vector3 &pos, const int n=10, const Vector3b col={255,255,0});
    void AddCoord(const Vector3 &pos, const Vector3 &axis={0,0,1}, const int n=10);
    Vector2d get_text_size(const std::string &str, float size) const;
#define VKUI_USE_CLIPPING_RECT // TODO: 最終的にはこれなしで動くようにする
    // [ SECTION ] --------  2D Rendering functions ----------
    // VKUI_USE_CLIPPING_RECTがdefineされているときはクリッピングする
    // TODO: stackにしてpop/pushするべきかも
    void __AddPointSizeZero2D(const Vector2d &pos, const Vector3b &col);
    inline void __AddPointSizeZero2D(const Vector2d &pos, const Vector3b &col, const Vector2d &uv ){ vertices_ui.push_back(std::move(VertexUI(pos, col, uv))); }

    inline void AddTriangle2D(
        const Vector2d &pos1, const Vector2d &pos2, const Vector2d &pos3, 
        const Vector3b &col1, const Vector3b &col2, const Vector3b &col3, 
        const Vector2d &uv1, const Vector2d &uv2, const Vector2d &uv3){
        __AddPointSizeZero2D(pos1, col1, uv1); __AddPointSizeZero2D(pos3, col3, uv3); __AddPointSizeZero2D(pos2, col2, uv2); 
    }
    inline void AddTriangle2D(
        const Vector2d &pos1, const Vector2d &pos2, const Vector2d &pos3, 
        const Vector3b &col1, const Vector3b &col2, const Vector3b &col3) { 
      __AddPointSizeZero2D(pos1, col1);  __AddPointSizeZero2D(pos3, col3); __AddPointSizeZero2D(pos2, col2);
    }
    // TODO: クリッピングする実装
    inline void AddTriangle2D(
        const Vector2d &pos1, const Vector2d &pos2, const Vector2d &pos3, 
        const Vector2d uv1, const Vector2d &uv2, const Vector2d &uv3, const Vector3b &col){ 
      __AddPointSizeZero2D(pos1, col, uv1); __AddPointSizeZero2D(pos3, col, uv3); __AddPointSizeZero2D(pos2, col, uv2);
    }
    inline void AddTriangle2D(
        const Vector2d &pos1, const Vector2d &pos2, const Vector2d &pos3, const Vector3b &col) { 
      __AddPointSizeZero2D(pos1, col); __AddPointSizeZero2D(pos3, col); __AddPointSizeZero2D(pos2, col); 
    }

    inline void AddQuad2D(
        const Vector2d &pos1, const Vector2d &pos2, const Vector2d &pos3, const Vector2d &pos4, 
        const Vector2d &uv1, const Vector2d &uv2, const Vector2d &uv3, const Vector2d &uv4, 
        const Vector3b &col){
        AddTriangle2D(pos1, pos2, pos3, uv1, uv2, uv3, col);
        AddTriangle2D(pos1, pos3, pos4, uv1, uv3, uv4, col); 
    }
    inline void AddQuad2D(const Vector2d &p1, const Vector2d &p2, const Vector2d &p3, const Vector2d &p4, const Vector3b &col){
      AddTriangle2D(p1, p2, p3, col);
      AddTriangle2D(p1, p3, p4, col); 
    }

    inline void AddRectTB2D(const Vector2d &top, const Vector2d& btm, const Vector3b &col){
        const Vector2d pos[4] = { 
            top,
            Vector2d(top[0], btm[1]),
            btm,
            Vector2d(btm[0], top[1]),
        };
        AddTriangle2D(pos[0], pos[1], pos[2], col);
        AddTriangle2D(pos[0], pos[2], pos[3], col);
    }

    inline void AddRectTB2D(const Vector2d &top, const Vector2d& btm, const Vector3b &col, const float width){
        const Vector2d pos[4] = { 
            top,
            Vector2d(top[0], btm[1]),
            btm,
            Vector2d(btm[0], top[1]),
        };
        AddLine2D(pos[0], pos[1], col, width);
        AddLine2D(pos[1], pos[2], col, width);
        AddLine2D(pos[2], pos[3], col, width);
        AddLine2D(pos[3], pos[0], col, width);
    }
    inline void AddRectPosSize(const Vector2d &pos, const Vector2d& size, const Vector3b &col){
        AddRectTB2D(pos, pos + size, col);
    }
    void AddRotatedRectPosSize(const Vector2d &pos, const Vector2d& size, const double theta, const Vector3b &col);
    void AddRotatedRectPosSize(const Vector2d &pos, const Vector2d& size, const double theta, const Vector3b &col, const float width=2.0f);
    inline void AddRectPosSize(const Vector2d &pos, const Vector2d& size, const Vector3b &col, float width){
        AddRectTB2D(pos, pos + size, col, width);
    }
    inline void AddLine2D(const Vector2d &pos1, const Vector2d &pos2, const Vector3b &col1, const Vector3b &col2, const float width = 1.0){
        // const auto dpd = pos2 - pos1; 
        // const auto dp = Vector2{ (double)dpd[0], (double)dpd[1] } / dpd.norm();
        const auto dp = pos2 - pos1;
        const Vector2d e = { (int)(dp[1]*width/ dp.norm()), -(int)(dp[0]*width/ dp.norm() )};
        AddTriangle2D(pos1, pos2,   pos2+e, col1, col2, col2); AddTriangle2D(pos1,   pos2+e, pos2, col1, col2, col2);
        AddTriangle2D(pos1, pos2+e, pos1+e, col1, col2, col1); AddTriangle2D(pos1, pos1+e, pos2+e, col1, col2, col1);
    }
    inline void AddLine2D(const Vector2d &pos1, const Vector2d &pos2, const Vector3b &col, const float width = 1.0){ AddLine2D(pos1,pos2, col, col, width); }
    void AddArrow2D(const Vector2d &from, const Vector2d &to, const Vector3b &col, const float width = 1.0);

    inline void AddQuad2D(const Vector2d &pos1, const Vector2d &pos2, const Vector2d &pos3, const Vector2d &pos4, const Vector3b &col, const float width){
        AddLine2D(pos1, pos2, col, width); 
        AddLine2D(pos2, pos3, col, width); 
        AddLine2D(pos3, pos4, col, width); 
        AddLine2D(pos4, pos1, col, width);
    }
    
    void AddCheck2D(const Vector2d &pos, const int size, const Vector3b & col, float line_width){
        AddLine2D(pos + Vector2({(float)size*0.1, (float)size*0.5}), pos + Vector2{(float)size*0.5, (float)size*0.9}, col, line_width);
        AddLine2D(pos + Vector2({(float)size*0.5, (float)size*0.9}), pos + Vector2{(float)size*0.9, (float)size*0.1}, col, line_width);
    }

    void AddArrowDown2D(const Vector2d &pos, const int size, const Vector3b & col){
        const int h = (float)size * 0.85f; AddTriangle2D(pos, pos + Vector2d{size/2, h}, pos + Vector2d{size, 0}, col);
    }

    void AddArrowUp2D(const Vector2d &pos, const int size, const Vector3b & col){
        const int h = (float)size * 0.85f; AddTriangle2D(pos+Vector2d{0, h}, pos + Vector2d{size, h}, pos + Vector2d{size/2, 0}, col);
    }

    void AddArrowLeft2D(const Vector2d &pos, const int size, const Vector3b & col){
       const int h = (float)size * 0.85f;  AddTriangle2D(pos+Vector2d{0, size/2}, pos + Vector2d{h, size}, pos + Vector2d{h, 0}, col);
    }

    void AddArrowRight2D(const Vector2d &pos, const int size, const Vector3b & col){
        const int h = (float)size * 0.85f;   AddTriangle2D(pos+Vector2d{0, size}, pos + Vector2d{h, size}, pos + Vector2d{h, 0}, col);
    }
    void AddCrossButton(const Vector2d &pos, const int size, const Vector3b &bg_col, const Vector3b &line_col, const Vector3b &cross_col){
        AddRectTB2D(pos, pos + size, bg_col);
        AddRectTB2D(pos, pos + size, line_col, 1);
        constexpr int _padding_cross = 2;
        constexpr int _cross_line_width = 2;
        AddLine2D(pos + _padding_cross, pos+size-_padding_cross, {255, 255, 255}, _cross_line_width);
        AddLine2D(pos + Vector2d{size - _padding_cross, _padding_cross}, pos+Vector2d{_padding_cross, size-_padding_cross}, cross_col, _cross_line_width);
    }

    inline void AddCross2D(const Vector2d &center, const int size, const Vector3b &col){
      constexpr int _cross_line_width = 2;
      const Vector2d pos = center - size/2;
      AddLine2D(pos, pos+size, col, _cross_line_width);
      AddLine2D(pos + Vector2d{size, 0}, pos+Vector2d{0, size}, col, _cross_line_width);
    }

    inline void AddPlus2D(const Vector2d &center, const int size, const Vector3b &col){
      constexpr int _cross_line_width = 2;
      const int hs = size/2; // half size
      AddLine2D(center - Vector2d(hs, 0), center + Vector2d(hs, 0), col, _cross_line_width);
      AddLine2D(center - Vector2d(0, hs), center + Vector2d(0, hs), col, _cross_line_width);
    }

    inline void AddDiamond2D(const Vector2d &center, const int size, const Vector3b &col){
      const int hs = size/2;
      AddQuad2D(center + Vector2d{hs, 0}, center + Vector2d{0, hs}, center - Vector2d{hs, 0}, center - Vector2d{0, hs}, col);
    }

    void AddCircle2D(const Vector2d &pos, const int size, const Vector3b col){
      if(size < 15){
        constexpr double s45 = 0.707090402;
        const Vector2 p[8] = {
          {0,   1}, {s45, s45}, {1,   0}, {s45, -s45},
          {0,  -1}, {-s45, -s45}, {-1,   0}, {-s45, s45}
        };
        for(int i=0; i<6; i++) AddTriangle2D(pos+p[0]*size, pos+p[1+i]*size, pos+p[i+2]*size, col);
      }else{
        constexpr double dth = 6.28 / 20.0f;
        for(int i=0; i<20; i++){
          const auto p1 = pos + Vector2d(std::cos(dth*i)*size, std::sin(dth*i) * size);
          const auto p2 = pos + Vector2d(std::cos(dth*(i+1))*size, std::sin(dth*(i+1)) * size);
          AddTriangle2D(pos, p1, p2, col);
        } 
      }
    }

    void AddCircle2D(const Vector2d &pos, const int size, const Vector3b col, const int width){
      constexpr double dth = 6.28 / 20.0f;
      for(int i=0; i<20; i++){
        const auto p1 = pos + Vector2d(std::cos(dth*i)*size, std::sin(dth*i) * size);
        const auto p2 = pos + Vector2d(std::cos(dth*(i+1))*size, std::sin(dth*(i+1)) * size);
        AddLine2D(p1, p2, col, width);
      } 
    }
#ifdef VKUI_USE_CLIPPING_RECT
    inline void AddQuad2D_clip(
        const Vector2d &pos1, const Vector2d &pos2, const Vector2d &pos3, const Vector2d &pos4, 
        const Vector2d &uv1, const Vector2d &uv2, const Vector2d &uv3, const Vector2d &uv4, 
        const Vector3b &col){
      const Vector2d pos1_{
        std::clamp<int>(pos1[0], clipping_rect.posX, clipping_rect.right()), 
        std::clamp<int>(pos1[1], clipping_rect.posY, clipping_rect.bottom()), 
      };
      const Vector2d pos2_{
        std::clamp<int>(pos2[0], clipping_rect.posX, clipping_rect.right()), 
        std::clamp<int>(pos2[1], clipping_rect.posY, clipping_rect.bottom()), 
      };
      const Vector2d pos3_{
        std::clamp<int>(pos3[0], clipping_rect.posX, clipping_rect.right()), 
        std::clamp<int>(pos3[1], clipping_rect.posY, clipping_rect.bottom()), 
      };
      const Vector2d pos4_{
        std::clamp<int>(pos4[0], clipping_rect.posX, clipping_rect.right()), 
        std::clamp<int>(pos4[1], clipping_rect.posY, clipping_rect.bottom()), 
      };
        AddTriangle2D(pos1_, pos2_, pos3_, uv1, uv2, uv3, col);
        AddTriangle2D(pos1_, pos3_, pos4_, uv1, uv3, uv4, col); 
    }

    inline void AddRectTB2D_clip(const Vector2d &top, const Vector2d& btm, const Vector3b &col){
        const Vector2d top_clipped{
          std::max<int>(top[0], clipping_rect.posX),
          std::max<int>(top[1], clipping_rect.posY)
        };
        const Vector2d btm_clipped{
          std::min<int>(btm[0], clipping_rect.right()),
          std::min<int>(btm[1], clipping_rect.bottom())
        };
        const Vector2d pos[4] = { 
            top_clipped,
            Vector2d(top_clipped[0], btm_clipped[1]),
            btm_clipped,
            Vector2d(btm_clipped[0], top_clipped[1]),
        };
        AddTriangle2D(pos[0], pos[1], pos[2], col);
        AddTriangle2D(pos[0], pos[2], pos[3], col);
    }

    inline void AddRectTB2D_clip(const Vector2d &top, const Vector2d& btm, const Vector3b &col, const float width){
        const Vector2d top_clipped{
          std::max<int>(top[0], clipping_rect.posX),
          std::max<int>(top[1], clipping_rect.posY)
        };
        const Vector2d btm_clipped{
          std::min<int>(btm[0], clipping_rect.right()),
          std::min<int>(btm[1], clipping_rect.bottom())
        };
        const Vector2d pos[4] = { 
            top_clipped,
            Vector2d(top_clipped[0], btm_clipped[1]),
            btm_clipped,
            Vector2d(btm_clipped[0], top_clipped[1]),
        };
        AddLine2D(pos[0], pos[1], col, width);
        AddLine2D(pos[1], pos[2], col, width);
        AddLine2D(pos[2], pos[3], col, width);
        AddLine2D(pos[3], pos[0], col, width);
    }

    inline void AddRectPosSize_clip(const Vector2d &pos, const Vector2d& size, const Vector3b &col){
        AddRectTB2D_clip(pos, pos + size, col);
    }
    inline void AddRectPosSize_clip(const Vector2d &pos, const Vector2d& size, const Vector3b &col, float width){
        AddRectTB2D_clip(pos, pos + size, col, width);
    }
#endif

    Vector2d AddString2D(const std::string &str, const Vector2d &pos, const float size, const Vector3b &col={255,255,255}, const int xlim=std::numeric_limits<int>::max());
    Vector2d AddString2D(const std::string &str, const Vector2d &pos, const float size, const uiRect &clip_rect, const Vector3b &col={255,255,255});

    inline void setCameraPos(const Vector3 pos){ camera_position.pos = pos; }
    inline void setCameraTarget(const Vector3 target){ camera_position.dir = target; }
    inline auto getCameraPos() const { return  camera_position.pos; }
    inline auto getCameraTarget() const { return camera_position.dir; }
    inline void setCameraUpVector(const Vector3 up){ camera_position.u = up; }
    inline void setCameraScale(const float scale){camera_position.scale = scale;}
    inline void setCameraZclip(const double near, const double far){camera_position.zNear = near; camera_position.zFar = far;}
    inline void setCameraAspect(const double aspect){camera_position.aspect = aspect;}
    void terminate(); 
};


// -----------------------------------------------------
//    [SECTION] uiFont
// -----------------------------------------------------


// 1つの文字（グリフ）に対するデータ
struct uiGlyph{
    // unsigned int    Codepoint : 31;     // 0x0000..0xFFFF
    // unsigned int    Visible : 1;        // Flag to allow early out when rendering
	unsigned int    U0, V0, U1, V1;     // UVテクスチャの座標
	int    dHeight;            // グリフの最上点から基準ラインまでの高さ
};


enum class FontLanguage{
	Japansese,
	English,
	Chinese,
	Korean,
	Thai,
	Vietnamese,
	// JPN,ENG,CHI,
};

	
using uiWchar = unsigned short;

class uiFont {
// private:
public:
	FT_Library library;
	FT_Face face;
	FT_GlyphSlot slot;  // グリフへのショートカット

	// uiFontFlags    flags;
	unsigned short    desiredTextSize;

	float             Spacing;            // 字間、config()関数内でuiStyle->FontSpacingの値が設定される
	float             FontSize;           // フォントサイズ、config()関数内でuiStyle->FontSpacingの値が設定される　
	unsigned int      TexWidth;           // build()関数で作成される文字テクスチャのサイズ＜GL_MAX_TEXTURE_SIZE.
	unsigned int      TexHeight;          // build()関数で作成される文字テクスチャのサイズ＜GL_MAX_TEXTURE_SIZE.
	unsigned int      TexHeight_capacity; // 予約済み　のテクスちゃ老域の全体高さ
	bool              isBuildFinished;    // フォントの作成が終わったかどうか
	FontLanguage      language;
	uiWchar*          GlyphRanges;
	unsigned int      nGlyph;             //GlyphRangesの配列の数（つまりグリフの数）
	std::string       FontName;

	//bool              MouseCursorIcons;   // 
	Vector2d           TexUvWhitePixel;    // Texture coordinates to a white pixel
    uiVector<uiWchar> IndexLookup;        // 12-16 // out //            // Sparse. Index glyphs by Unicode code-point.
    uiVector<uiGlyph> Glyphs;             // すべての文字に対するグリフ
    uiGlyph*          FallbackGlyph;      //FinGlyphで上手くいかなかったときのGlyph（□の文字化けのやつ）
	
	unsigned char *_Data; // テクスチャデータ
    //float Scale;
	void              AddGlyph(uiWchar c, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x);
	void              AddRemapChar(uiWchar dst, uiWchar src, bool overwrite_dst = true); // Makes 'dst' character/glyph points to 'src' character/glyph. Currently needs to be called AFTER fonts have been built.
	void              SetGlyphVisible(uiWchar c, bool visible);
	bool              IsGlyphRangeUnused(unsigned int c_begin, unsigned int c_last);

    uiWchar*    GetGlyphRangesKorean();
    uiWchar*    GetGlyphRangesJapanese();
    uiWchar*    GetGlyphRangesEnglish();
    uiWchar*    GetGlyphRangesChinese();
    uiWchar*    GetGlyphRangesCyrillic(); 
    uiWchar*    GetGlyphRangesThai(); 
    uiWchar*    GetGlyphRangesVietnamese();

public:
	uiFont();
	~uiFont();
	void              init();
	bool              setLanguage(FontLanguage l);
	void              setStyle(uiStyle *style);
	bool              build();                           //フォンﾄトをレンダリングする
    uiGlyph*          FindGlyph(uiWchar c);              //
	bool              getSize(uiWchar c, Vector2d *size, unsigned int *dH);
    Vector2f          CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end = NULL, const char** remaining = NULL) const; // utf8
    char*             CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const;
	
	inline unsigned int getTexWidth() { return TexWidth;  }
	inline unsigned int getTexHeight(){ return TexHeight; }
	inline unsigned char *getData()   { return _Data;     }
	inline unsigned int getGlyphNum() { return Glyphs.size();    }
    void setFontSize    (const int s);
    void setFontSpacing (const int s);
    void setFontName (const std::string &s);
};



// -----------------------------------------------------
//    [SECTION] Engine
// -----------------------------------------------------
struct uiEngine {
    vk::UniqueInstance instance;
    VkDebugUtilsMessengerEXT callback;
    vk::PhysicalDevice physicalDevice;
    vk::UniqueDevice device;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    std::vector<uiWindow *>windows;
    vk::DescriptorPool descriptorPool;
    vk::CommandPool commandPool;
    vk::Image TextureImage;
    vk::DeviceMemory textureImageMemory;
    vk::ImageView textureImageView;
    vk::Sampler textureSampler;
    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    vk::UniqueShaderModule vertexShader, fragmentShader, vertexShader_ui, fragmentShader_ui;

    uiWindow *drawingWnd, *hoveringWnd, *focusedWnd;
	uiFont    text_renderer;
    uiStyle style;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    void initVulkan() {
        createInstance();
        setupDebugCallback();
        for(int i=0; i<windows.size(); i++){ windows[i]->createSurface(); }
        // createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createDescriptorPool();
        createCommandPool();
        createShaders();
        createSyncObjects();
        text_renderer.init();
        text_renderer.setLanguage(FontLanguage::Japansese);
        text_renderer.build();
        createTextureImage();
        for(int i=0; i<windows.size(); i++){ windows[i]->init(); }
        std::cout << "createSyncObjects" << std::endl;
    }

    void createTextureImage();
    void transitionImageLayout(vk::Image image, [[maybe_unused]]vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
        vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = (vk::AccessFlags)0;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout ==vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), {}, {}, barrier);
        endSingleTimeCommands(commandBuffer);
    }

    vk::CommandBuffer beginSingleTimeCommands() {
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;
        vk::CommandBuffer cmdbuf;
        try {
            auto _commandBuffers = device->allocateCommandBuffers(allocInfo);
            assert(_commandBuffers.size() > 0);
            cmdbuf = _commandBuffers[0];
        } catch (const vk::SystemError &err) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        auto result = cmdbuf.begin(&beginInfo);
        if(result != vk::Result::eSuccess){
            throw std::runtime_error("failed to begine command buffer!");
        }
        std::cout << "begin" << std::endl;
        return cmdbuf;
    }

    void endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
        commandBuffer.end();
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        graphicsQueue.submit(submitInfo, nullptr);
        graphicsQueue.waitIdle();
        device->freeCommandBuffers(commandPool, 1, &commandBuffer);
    }

    bool update(const bool verbose) {
        assert(windows.size()> 0);
        glfwPollEvents();
        for(int i=0; i<windows.size(); i++){ windows[i]->drawFrame(verbose); }
        device->waitIdle();
        return !windows[0]->wndShouldClose();
    }

    void cleanup() {
        // NOTE: instance destruction is handled by UniqueInstance, same for device
        // cleanupSwapChain();
        // for(int i=0; i<windows.size(); i++;){windows[i]->cleanupSwapChainU(); } // TODO: cleanupSwapChainU

        for (size_t i = 0; i < VK_ENGINE_MAX_FRAMES_IN_FLIGHT; i++) {
            device->destroySemaphore(renderFinishedSemaphores[i]);
            device->destroySemaphore(imageAvailableSemaphores[i]);
            device->destroyFence(inFlightFences[i]);
        }

        device->destroyCommandPool(commandPool);

        // surface is created by glfw, therefore not using a Unique handle
        // instance->destroySurfaceKHR(surface); // TODO: 

#ifdef VK_ENGINE_ENABLE_VALIDATION_LAUERS
            DestroyDebugUtilsMessengerEXT(*instance, callback, nullptr);
#endif
        // glfwDestroyWindow(window);  // TODO: 
        glfwTerminate();
    }

    void terminate(const bool verbose=false){
       if(verbose){ std::cout << "terminating vkui......." << std::endl; }
       cleanup();
       if(verbose){ std::cout << "deleting windows ........" << std::endl; }
       for(int i=0; i<windows.size(); i++){ windows[i]->terminate(); }
       if(verbose){ std::cout << "deleting windows ........" << std::endl; }
       return;
    }

    void createCommandPool();
    std::vector<const char*> getRequiredExtensions();
    vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);
    bool isDeviceSuitable(const vk::PhysicalDevice& device);
    bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device);
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
    bool checkValidationLayerSupport() ;
    void createSyncObjects();
    void createDescriptorPool();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createInstance();
    void setupDebugCallback();
    vk::UniqueShaderModule createShader(std::string filename);

    void updateVertexBuffer();

    void createShaders(){
        vertexShader = createShader("../shaders/vert.spv");
        fragmentShader = createShader("../shaders/frag.spv");

        vertexShader_ui = createShader("../shaders/vert_ui.spv");
        fragmentShader_ui = createShader("../shaders/frag_ui.spv");
    }
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        [[maybe_unused]]VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        [[maybe_unused]]VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]]void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
};

#define VKUI_ENGINE_API 

extern uiEngine engine;
inline VKUI_ENGINE_API uiEngine *         getContextPtr(){return &engine;}
inline VKUI_ENGINE_API vk::UniqueDevice *  getDevicePtr(){return &(engine.device);}
inline VKUI_ENGINE_API vk::PhysicalDevice *getPhysicalDevicePtr(){return &(engine.physicalDevice);}
inline VKUI_ENGINE_API vk::UniqueInstance *getInstancePtr(){return &(engine.instance);}
inline VKUI_ENGINE_API vk::Queue *         getGraphicsQueuePtr(){return &(engine.graphicsQueue);}
inline VKUI_ENGINE_API vk::Queue *         getPresentQueuePtr(){return &(engine.presentQueue);}
inline VKUI_ENGINE_API auto getVertexShaderPtr(){return &(engine.vertexShader);}
inline VKUI_ENGINE_API auto getFragmentShaderPtr(){return &(engine.fragmentShader);}

inline VKUI_ENGINE_API auto getVertexShaderUIPtr(){return &(engine.vertexShader_ui);}
inline VKUI_ENGINE_API auto getFragmentShaderUIPtr(){return &(engine.fragmentShader_ui);}

inline VKUI_ENGINE_API vk::CommandPool *   getCommandPool(){return &(engine.commandPool);}
inline VKUI_ENGINE_API vk::DescriptorPool *getDesctiptorPoolPtr(){return &(engine.descriptorPool);}
inline VKUI_ENGINE_API auto getImageAvailableSemaphores(){return &(engine.imageAvailableSemaphores);}
inline VKUI_ENGINE_API auto getRenderFinishedSemaphores(){return &(engine.renderFinishedSemaphores);}
inline VKUI_ENGINE_API auto getInFlightFences(){return &(engine.inFlightFences);}
inline VKUI_ENGINE_API auto getTextureSampler(){return &(engine.textureImageView);}
inline VKUI_ENGINE_API auto getTextureImageView(){return &(engine.textureSampler);}

inline VKUI_ENGINE_API auto setDrawingWindow(uiWindow *wnd){return engine.drawingWnd = wnd;}
inline VKUI_ENGINE_API auto getDrawingWindow(){return engine.drawingWnd;}
inline VKUI_ENGINE_API auto setFocusedWindow(uiWindow *wnd){return engine.focusedWnd = wnd;}
inline VKUI_ENGINE_API auto getFocusedWindow(){return engine.focusedWnd;}
inline VKUI_ENGINE_API auto setHoveringWindow(uiWindow *wnd){return engine.hoveringWnd = wnd;}
inline VKUI_ENGINE_API auto getHoveringWindow(){return engine.hoveringWnd;}

// inline VKUI_ENGINE_API auto setFocusedWidget(uiWidget *w){return engine.focusedWidget = w;}
inline VKUI_ENGINE_API uiWidget *getFocusedWidget(){return getDrawingWindow()->root_widget_ui.getFocusedWidget();}
inline VKUI_ENGINE_API uiWidget *getHoveringWidget(){return getDrawingWindow()->root_widget_ui.getHoveringWidget();}

inline VKUI_ENGINE_API auto getTextRendererPtr(){ return &(engine.text_renderer); };


// initialize uiContext and create default window
inline VKUI_ENGINE_API void init(){ engine.initWindow(); }
inline VKUI_ENGINE_API void initFinish(){ assert(engine.windows.size() > 0); engine.initVulkan();}
inline VKUI_ENGINE_API uiWindow *addWindow(std::string name, int w, int h){ const auto A = new uiWindow(name, w,h); engine.windows.push_back(A); return A; }
inline VKUI_ENGINE_API bool render(const bool verbose = false){return engine.update(verbose);}
inline VKUI_ENGINE_API void Terminate(const bool verbose = false){engine.terminate(verbose);}

// styles
inline VKUI_ENGINE_API const uiStyle *getStyle() { return &engine.style; }
inline VKUI_ENGINE_API void setStyle(const uiStyle &s){ engine.style = s; }

} // nsmespace vkUI::Engine

