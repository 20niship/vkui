#include <engine.hpp>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <type_traits>
#include <widget.hpp>
#ifdef DEFINED_STD_FORMAT
#include <format>
#else
#include <functional>
#include <iostream>
#include <logger.hpp>
#include <string>
#include <utility>

template <typename... Args> std::string myFormat(const std::string& base_str, const Args... args) {
  std::string base_str_ = base_str;
  /* constexpr std::size_t size = sizeof...(Args); */
  auto tt = std::make_tuple(args...);
  std::apply(
    [&](auto&&... args_) {
      ((base_str_.replace(base_str_.find("{}") != std::string::npos ? base_str_.find("{}") : 0, base_str_.find("{}") == std::string::npos ? 0 : 2, std::to_string(args_))), ...);
    },
    tt);
  return base_str_;
}
#endif
#include <chrono>

namespace vkUI::Engine {
uiWindow::uiWindow(std::string _name, uint16_t width, uint16_t height) {
  window = glfwCreateWindow(width, height, _name.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, resizeCB_static);
  glfwSetCursorPosCallback(window, mouseCB_static);
  glfwSetKeyCallback(window, keyboardCB_static);
  glfwSetScrollCallback(window, scrollCB_static);
  glfwSetCharCallback(window, charCB_static);
  glfwSetMouseButtonCallback(window, mouseButtonCB_static);
  currentFrame = 0;

  user_key_cb = [](int a, int b, int c, int d, Vector2 f) { return true; };
}

SwapChainSupportDetails uiWindow::querySwapChainSupport(const vk::PhysicalDevice& device) {
  SwapChainSupportDetails details;
  details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
  details.formats = device.getSurfaceFormatsKHR(surface);
  details.presentModes = device.getSurfacePresentModesKHR(surface);

  return details;
}

void uiWindow::createSurface() {
  auto instance = getInstancePtr();
  VkSurfaceKHR rawSurface;
  if(glfwCreateWindowSurface((*instance).get(), window, nullptr, &rawSurface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }

  surface = rawSurface;
}

void uiWindow::createSwapChain() {
  auto device = getDevicePtr();
  auto physicalDevice = getPhysicalDevicePtr();
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(*physicalDevice);

  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo(vk::SwapchainCreateFlagsKHR(), surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent,
                                        1, // imageArrayLayers
                                        vk::ImageUsageFlagBits::eColorAttachment);
  auto engine = getContextPtr();
  QueueFamilyIndices indices = engine->findQueueFamilies(*physicalDevice);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if(indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

  try {
    swapChain = (*device)->createSwapchainKHR(createInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create swap chain!");
  }

  swapChainImages = (*device)->getSwapchainImagesKHR(swapChain);

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void uiWindow::createImageViews() {
  auto device = getDevicePtr();
  swapChainImageViews.resize(swapChainImages.size());

  for(size_t i = 0; i < swapChainImages.size(); i++) {
    vk::ImageViewCreateInfo createInfo = {};
    createInfo.image = swapChainImages[i];
    createInfo.viewType = vk::ImageViewType::e2D;
    createInfo.format = swapChainImageFormat;
    createInfo.components.r = vk::ComponentSwizzle::eIdentity;
    createInfo.components.g = vk::ComponentSwizzle::eIdentity;
    createInfo.components.b = vk::ComponentSwizzle::eIdentity;
    createInfo.components.a = vk::ComponentSwizzle::eIdentity;
    createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    try {
      swapChainImageViews[i] = (*device)->createImageView(createInfo);
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

void uiWindow::createRenderPass() {
  auto device = getDevicePtr();

  vk::AttachmentDescription colorAttachment = {};
  colorAttachment.format = swapChainImageFormat;
  colorAttachment.samples = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
  colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass = {};
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  vk::SubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  // dependency.srcAccessMask = 0;
  dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

  vk::RenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  try {
    renderPass = (*device)->createRenderPass(renderPassInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void uiWindow::createDescriptorSetLayout() {
  auto device = getDevicePtr();
  auto descriptorPool = getDesctiptorPoolPtr();
  auto textureImageView = getTextureSampler();
  auto textureSampler = getTextureImageView();

  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  bindings.resize(2);
  bindings[0].binding = 0;
  bindings[0].descriptorCount = 1;
  bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
  bindings[0].pImmutableSamplers = nullptr;
  bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;

  // CombinedImageSampler (Depth buffer) for PixelShader
  bindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
  bindings[1].descriptorCount = 1;
  bindings[1].binding = 1;
  bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;
  bindings[1].pImmutableSamplers = nullptr;

  // レイアウトを生成
  vk::DescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  descriptorSetLayout = (*device)->createDescriptorSetLayout(layoutInfo, nullptr);

  // デスクリプタセットは作成済みのデスクリプタプールから確保する
  vk::DescriptorSetAllocateInfo allocInfo{};
  allocInfo.descriptorPool = *descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  descriptorSet = (*device)->allocateDescriptorSets(allocInfo);

  // デスクリプタセットの情報を更新する
  vk::DescriptorBufferInfo g_VkUniformInfo;
  g_VkUniformInfo.buffer = uniformBuffer.buf;
  g_VkUniformInfo.offset = 0;
  g_VkUniformInfo.range = VK_WHOLE_SIZE; // sizeof(float)*16; // VK_WHOLE_SIZE

  vk::DescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = vk::ImageLayout::eGeneral;
  imageInfo.imageView = *textureImageView;
  imageInfo.sampler = *textureSampler;

  std::vector<vk::WriteDescriptorSet> descSetInfos;
  descSetInfos.resize(2);
  // descSetInfos.resize(1);
  descSetInfos[0].dstSet = descriptorSet[0];
  descSetInfos[0].descriptorCount = 1;
  descSetInfos[0].descriptorType = vk::DescriptorType::eUniformBuffer;
  descSetInfos[0].pBufferInfo = &g_VkUniformInfo;
  descSetInfos[0].dstArrayElement = 0;
  descSetInfos[0].dstBinding = 0;

  descSetInfos[1].dstSet = descriptorSet[0];
  descSetInfos[1].dstBinding = 1;
  descSetInfos[1].dstArrayElement = 0;
  descSetInfos[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
  descSetInfos[1].descriptorCount = 1;
  descSetInfos[1].pImageInfo = &imageInfo;
  try {
    (*device)->updateDescriptorSets(descSetInfos, nullptr);
  } catch(vk::SystemError& err) {
    std::cout << "vk::SystemError: " << err.what() << std::endl;
    exit(-1);
  } catch(std::exception& e) {
    std::cerr << "Exception caught : " << e.what() << std::endl;
  } catch(...) {
    std::cout << "errrrrrr" << std::endl;
    exit(-1);
  }
}

void uiWindow::createGraphicsPipeline() {
  auto device = getDevicePtr();
  auto vertexShader = getVertexShaderPtr();
  auto fragmentShader = getFragmentShaderPtr();

  vk::PipelineShaderStageCreateInfo shaderStages[] = {{vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, (*vertexShader).get(), "main"},
                                                      {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, (*fragmentShader).get(), "main"}};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
  // inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Viewoortに関する設定
  vk::PipelineViewportStateCreateInfo viewportState = {};
  // {
  vk::Viewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor = {};
  scissor.offset = vk::Offset2D(0, 0);
  scissor.extent = swapChainExtent;

  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;
  // }

  vk::PipelineRasterizationStateCreateInfo rasterizer = {};
  // {
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = vk::PolygonMode::eFill;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = vk::CullModeFlagBits::eBack;
  rasterizer.frontFace = vk::FrontFace::eClockwise;
  rasterizer.depthBiasEnable = VK_FALSE;
  // }

  vk::PipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineColorBlendStateCreateInfo colorBlending = {};
  // {
  vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable = VK_FALSE;

  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = vk::LogicOp::eCopy;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;
  // }

  // vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
  // pipelineLayoutInfo.setLayoutCount = 0;
  // pipelineLayoutInfo.pushConstantRangeCount = 0;

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout);
  // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  // pipelineLayoutInfo.setLayoutCount = 1;
  // pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

  try {
    pipelineLayout = (*device)->createPipelineLayout(pipelineLayoutInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  vk::GraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = nullptr;

  vk::ResultValue<vk::UniquePipeline> result = (*device)->createGraphicsPipelineUnique({}, pipelineInfo);
  if(result.result == vk::Result::eSuccess) {
    graphicsPipeline = std::move(result.value);
  } else {
    throw std::runtime_error("failed to create a pipeline!");
  }
}


void uiWindow::createGraphicsPipeline_UI() {
  auto device = getDevicePtr();
  auto vertexShader = getVertexShaderUIPtr();
  auto fragmentShader = getFragmentShaderUIPtr();

  vk::PipelineShaderStageCreateInfo shaderStages[] = {{vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, (*vertexShader).get(), "main"},
                                                      {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, (*fragmentShader).get(), "main"}};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

  auto bindingDescription = VertexUI::getBindingDescription();
  auto attributeDescriptions = VertexUI::getAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
  // inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Viewoortに関する設定
  vk::PipelineViewportStateCreateInfo viewportState = {};
  // {
  vk::Viewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vk::Rect2D scissor = {};
  scissor.offset = vk::Offset2D(0, 0);
  scissor.extent = swapChainExtent;

  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;
  // }

  vk::PipelineRasterizationStateCreateInfo rasterizer = {};
  // {
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = vk::PolygonMode::eFill;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = vk::CullModeFlagBits::eBack;
  rasterizer.frontFace = vk::FrontFace::eClockwise;
  rasterizer.depthBiasEnable = VK_FALSE;
  // }

  vk::PipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineColorBlendStateCreateInfo colorBlending = {};
  // {
  vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
  colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
  colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
  colorBlendAttachment.alphaBlendOp = vk::BlendOp::eSubtract;

  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = vk::LogicOp::eCopy;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;
  // }

  // vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
  // pipelineLayoutInfo.setLayoutCount = 0;
  // pipelineLayoutInfo.pushConstantRangeCount = 0;

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout);
  // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  // pipelineLayoutInfo.setLayoutCount = 1;
  // pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

  std::cout << "create createPipelineLayout" << std::endl;
  try {
    pipelineLayout_ui = (*device)->createPipelineLayout(pipelineLayoutInfo);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  vk::GraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipelineLayout_ui;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = nullptr;

  vk::ResultValue<vk::UniquePipeline> result = (*device)->createGraphicsPipelineUnique({}, pipelineInfo);
  if(result.result == vk::Result::eSuccess) {
    graphicsPipeline_ui = std::move(result.value);
  } else {
    throw std::runtime_error("failed to create a pipeline!");
  }
}



vk::SurfaceFormatKHR uiWindow::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
  if(availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
  }

  for(const auto& availableFormat : availableFormats) {
    if(availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }
  return availableFormats[0];
}

vk::PresentModeKHR uiWindow::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes) {
  vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

  for(const auto& availablePresentMode : availablePresentModes) {
    if(availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    } else if(availablePresentMode == vk::PresentModeKHR::eImmediate) {
      bestMode = availablePresentMode;
    }
  }
  return bestMode;
}

vk::Extent2D uiWindow::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
  if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
  }
}

void uiWindow::createFramebuffers() {
  auto device = getDevicePtr();
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for(size_t i = 0; i < swapChainImageViews.size(); i++) {
    vk::ImageView attachments[] = {swapChainImageViews[i]};

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    try {
      swapChainFramebuffers[i] = (*device)->createFramebuffer(framebufferInfo);
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void uiWindow::createCommandBuffers(bool force = false) {
  auto device = getDevicePtr();
  auto commandPool = getCommandPool();
  commandBuffers.resize(swapChainFramebuffers.size());

  static bool command_buffer_created = false;
  static size_t last_vertex_size = 0;
  static size_t last_vertex_ui_size = 0;

  if(!command_buffer_created) {
    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = *commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    try {
      commandBuffers = (*device)->allocateCommandBuffers(allocInfo);
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
    command_buffer_created = true;
  }

  if(last_vertex_size == vertices.size() && last_vertex_ui_size == dd.vertices_ui.size() && !force) return;

  last_vertex_size = vertices.size();
  last_vertex_ui_size = dd.vertices_ui.size();

  for(size_t i = 0; i < commandBuffers.size(); i++) {
    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

    try {
      commandBuffers[i].begin(beginInfo);
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = swapChainExtent;

    vk::ClearValue clearColor = {std::array<float, 4>{0.5f, 0.01f, 0.05f, 1.0f}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline.get());
    commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);
    // vk::Buffer vertexBuffers[] = { vertexBuffer.buf, vertexBuffer_ui.buf  };
    vk::Buffer vertexBuffers[] = {vertexBuffer.buf};
    vk::DeviceSize offsets[] = {0};
    commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
    commandBuffers[i].draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    // commandBuffers[i].draw(VK_WHOLE_SIZE, 1, 0, 0);

    commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline_ui.get());
    commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout_ui, 0, descriptorSet, nullptr);
    vk::Buffer vertexBuffers2[] = {vertexBuffer_ui.buf};
    commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers2, offsets);

    dd.push();
    dd.sort();
    for(auto&& d : dd.drawlist) {
      /* commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout_ui, d.tex_id, descriptorSet, nullptr); */
      commandBuffers[i].draw(static_cast<uint32_t>(dd.vertices_ui.size()), 1, 0, 0);
    }
    dd.clear();
    // for(int i=0; i<dd.vertices_ui.size(); i++){
    //     std::cout << dd.vertices_ui[i].pos[0] << ", " << vertices_ui[i].pos[1] << std::endl;
    // }
    commandBuffers[i].endRenderPass();
    try {
      commandBuffers[i].end();
    } catch(const vk::SystemError& err) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}

void uiWindow::renderUI() {
  setDrawingWindow(this);
  root_widget_ui.calcInnerSize_recursize();
  root_widget_ui.applyAlignment_recursive();
  // if(root_widget_ui.getNeedRendering()){
  root_widget_ui.needRendering(true);
  dd.vertices_ui.resize(0);
  root_widget_ui.render_child_widget();
  // }
  if(dd.vertices_ui.size() == 0) {
    dd.vertices_ui.push_back(VertexUI(Vector2{0, 0}, Vector3b{0, 0, 0}, Vector2{0, 0}));
    dd.vertices_ui.push_back(VertexUI(Vector2{0, 0}, Vector3b{0, 0, 0}, Vector2{0, 0}));
    dd.vertices_ui.push_back(VertexUI(Vector2{0, 0}, Vector3b{0, 0, 0}, Vector2{0, 0}));
  }
}

void uiWindow::updateVertexBuffer() {
  setDrawingWindow(this);

  vertices.resize(0);
  root_widget.render();
  renderUI();
  static bool _created_vertex_buffer = false;
  if(!_created_vertex_buffer) {
    // create 3d vertex buffer
    assert(vertices.size() > 0);
    assert(dd.vertices_ui.size() > 0);
    const vk::DeviceSize bufferSize = vertices.size_in_bytes();
    vertexBuffer.create(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    const vk::DeviceSize bufferSize2 = dd.vertices_ui.size_in_bytes();
    vertexBuffer_ui.create(bufferSize2, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    _created_vertex_buffer = true;
  }
}

void uiWindow::updateUniformBuffer() {
  static bool _created_uniform_buffer = false;
  const vk::DeviceSize bufferSize = sizeof(UniformData);
  if(!_created_uniform_buffer) {
    uniformBuffer.create(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    _created_uniform_buffer = true;
  }
  int ww, wh;
  glfwGetWindowSize(window, &ww, &wh);

  camera_position.getCameraProjMatrix(uniform_data.proj);
#if 0
    const float projectionMatrix[] = { // 左下原点
        2.0f / float(ww), 0.0f, 0.0f, 0.0f ,
        0.0f, -2.0f / float(wh), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
#else
  const float projectionMatrix[] = {// 左上原点
                                    2.0f / float(ww), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f / float(wh), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f};
#endif
  for(int i = 0; i < 16; i++) {
    uniform_data.proj_uv[i] = projectionMatrix[i];
  }
  const auto text_renderer = getTextRendererPtr();
  uniform_data.texure_size[0] = text_renderer->TexWidth;
  uniform_data.texure_size[1] = text_renderer->TexHeight;

  uniformBuffer.copyData(&uniform_data, bufferSize);
  // std::cout << std::fixed;
  //     for(int i=0; i<4; i++){
  //         for(int j=0; j<4; j++){std::cout << std::setprecision(3) << uniform_data.proj_uv[i*4+j] << ", "; }
  //         std::cout << std::endl;
  //     }
  // std::cout << "position = " << camera_position.pos << std::endl;
  // std::cout << "target = " << camera_position.dir << std::endl;
}

void uiWindow::drawDevelopperHelps() {
  const auto nCmd = commandBuffers.size();
  const auto wsize = root_widget_ui.get_widget_num();
#ifdef DEFINED_STD_FORMAT
  const std::string str = std::format(
#else
  const std::string str = myFormat(
#endif
    "vert_size = ({}, {}), drawCmd={} nWidget={}, fps={}", vertices.size(), dd.vertices_ui.size(), nCmd, wsize, fps);
  AddString2D(str, {10, 10}, 1, {255, 0, 255});

  const auto focused = root_widget_ui.getFocusedWidget();
  const auto hovering = root_widget_ui.getHoveringWidget();
  AddRectPosSize(hovering->getPos(), hovering->getSize(), {0, 255, 0}, 2);
  AddRectPosSize(focused->getPos(), focused->getSize(), {255, 0, 0}, 1);
}

void uiWindow::drawFrame(const bool verbose) {
  vertexBuffer.copyData((void*)vertices.data(), vertices.size_in_bytes());
  vertexBuffer_ui.copyData((void*)dd.vertices_ui.data(), dd.vertices_ui.size_in_bytes());
  if(verbose) {
    std::cout << "vertex size(3d, 2d, 2d_overlay) = " << vertices.size() << ", " << dd.vertices_ui.size() << std::endl;
  }
  auto currentFrame_2 = currentFrame % VK_ENGINE_MAX_FRAMES_IN_FLIGHT;
  setDrawingWindow(this);

  // updateVertexBuffer();
  updateUniformBuffer();
  createCommandBuffers();

#ifdef VKUI_ENGINE_ENABLE_FPS_CALC
  // static int frames_in_sec = 0;
  static std::chrono::system_clock::time_point start_time_fps = std::chrono::system_clock::now();
  const auto now = std::chrono::system_clock::now();
  const double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(start_time_fps - now).count();
  fps = (float)1e6 / elapsed;
  start_time_fps = now;
  // frames_in_sec++;
#endif

  auto device = getDevicePtr();
  auto presentQueue = getPresentQueuePtr();
  auto graphicsQueue = getGraphicsQueuePtr();
  auto imageAvailableSemaphores = getImageAvailableSemaphores();
  auto renderFinishedSemaphores = getRenderFinishedSemaphores();
  auto inFlightFences = getInFlightFences();
  assert(imageAvailableSemaphores->size() > currentFrame_2);
  assert(renderFinishedSemaphores->size() > currentFrame_2);
  assert(inFlightFences->size() > currentFrame_2);

  auto result = (*device)->waitForFences(1, &(*inFlightFences)[currentFrame_2], VK_TRUE, std::numeric_limits<uint64_t>::max());
  if(result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to waitForFences!");
  }
  uint32_t imageIndex;
  try {
    auto result = (*device)->acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), (*imageAvailableSemaphores)[currentFrame_2], nullptr);
    imageIndex = result.value;
  } catch(const vk::OutOfDateKHRError& err) {
    recreateSwapChain();
    return;
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vk::SubmitInfo submitInfo = {};

  vk::Semaphore waitSemaphores[] = {(*imageAvailableSemaphores)[currentFrame_2]};
  vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

  vk::Semaphore signalSemaphores[] = {(*renderFinishedSemaphores)[currentFrame_2]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;
  result = (*device)->resetFences(1, &(*inFlightFences)[currentFrame_2]);
  if(result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to submit resetFences!");
  }

  try {
    graphicsQueue->submit(submitInfo, (*inFlightFences)[currentFrame_2]);
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  vk::PresentInfoKHR presentInfo = {};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  vk::SwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  vk::Result resultPresent;
  try {
    resultPresent = presentQueue->presentKHR(presentInfo);
  } catch(const vk::OutOfDateKHRError& err) {
    resultPresent = vk::Result::eErrorOutOfDateKHR;
  } catch(const vk::SystemError& err) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  if(resultPresent == vk::Result::eSuboptimalKHR || resultPresent == vk::Result::eSuboptimalKHR || framebufferResized) {
    framebufferResized = false;
    recreateSwapChain();
    return;
  }
  currentFrame++;
}


void uiWindow::cleanupSwapChain() {
  auto device = getDevicePtr();
  // auto commandPool = getCommandPool();
  for(auto framebuffer : swapChainFramebuffers) {
    (*device)->destroyFramebuffer(framebuffer);
  }

  // (*device)->freeCommandBuffers(*commandPool, commandBuffers);

  // (*device)->destroyPipeline(graphicsPipeline);
  (*device)->destroyPipelineLayout(pipelineLayout);
  (*device)->destroyRenderPass(renderPass);

  for(auto imageView : swapChainImageViews) {
    (*device)->destroyImageView(imageView);
  }

  (*device)->destroySwapchainKHR(swapChain);
}

void uiWindow::recreateSwapChain() {
  auto device = getDevicePtr();

  int width = 0, height = 0;
  while(width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }

  (*device)->waitIdle();

  cleanupSwapChain();

  createSwapChain();
  createImageViews();
  createRenderPass();
  createGraphicsPipeline();
  createGraphicsPipeline_UI();
  createFramebuffers();
  createCommandBuffers(true);
}

void uiWindow::init() {
  createSwapChain();
  createImageViews();
  updateUniformBuffer();
  updateVertexBuffer();
  createDescriptorSetLayout();
  createRenderPass();
  createGraphicsPipeline();
  createGraphicsPipeline_UI();
  createFramebuffers();
  createCommandBuffers();
  cursors.init();
  root_widget_ui.needRendering(true);
  root_widget_ui.impl_needCalcAlignment_child();
  root_widget_ui.impl_needCalcInnerSize_parent();
}

void uiWindow::mouseCB(double x, double y) {
  static bool wnd_rotating = false;
  // static Vector3 start_pos_mouse, start_pos_camera;
  static double radius;
  static Vector2 rotation, start_pos_mouse;
  int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
  if(state == GLFW_PRESS) {
    const auto vecc = camera_position.pos - camera_position.dir;
    if(!wnd_rotating) {
      radius = vecc.norm();
      rotation[0] = std::acos(vecc[2] / radius);  // theta
      rotation[1] = std::atan2(vecc[1], vecc[0]); // phi
      start_pos_mouse = {x, y};
      // start_pos_camera = camera_position.pos;
    }
    const auto delta = Vector2(x, y) - start_pos_mouse;
    const double theta = rotation[0] + delta[0] / 250;
    const double phi = rotation[1] + delta[1] / 250;
    const Vector3 tmp = {radius * std::sin(theta) * std::cos(phi), radius * std::sin(theta) * std::sin(phi), radius * std::cos(theta)};
    camera_position.pos = tmp + camera_position.dir;
    wnd_rotating = true;
  } else if(wnd_rotating) {
    wnd_rotating = false;
  }
  int state_ = 0;
  state_ |= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
  state_ |= (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) << 1;
  state_ |= (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) << 2;
  root_widget_ui.CallbackFunc(uiCallbackFlags::MouseMove, {(int)x, (int)y}, 0, state_, nullptr);
}

void uiWindow::scrollCB(double xoffset, double yoffset) {
  std::cout << "scroll " << xoffset << ", " << yoffset << std::endl;
  camera_position.go_closer(yoffset);
  root_widget_ui.CallbackFunc(uiCallbackFlags::MouseScroll, {0, 0}, xoffset, yoffset, nullptr);
}


// ----------------------------------------------------------------
//     [ SECTION ] Drawing Functions
// ----------------------------------------------------------------

void uiWindow::AddLine(const Vector3& pos1, const Vector3& pos2, const Vector3b& col1, const Vector3b& col2, const float width) {
  auto dp = pos2 - pos1;
  dp = dp / dp.norm();
  Vector3 e1, e2;
  if(dp[0] >= dp[1] && dp[0] >= dp[2]) { // X軸に近い直線
    e1 = {dp[1], dp[0], dp[2]};
    e2 = {dp[2], dp[1], dp[0]};
  } else if(dp[1] >= dp[0] && dp[1] >= dp[2]) {
    e1 = {dp[1], dp[0], dp[2]};
    e2 = {dp[0], dp[2], dp[1]};
  } else {
    e1 = {dp[2], dp[1], dp[0]};
    e2 = {dp[0], dp[2], dp[1]};
  }
  e1 *= width;
  e2 *= width;
  AddTriangle(pos1, pos1 + e1 + e2, pos2 + e1, col1, col1, col2);
  AddTriangle(pos1 + e1 + e2, pos2 + e2, pos2 + e1, col1, col2, col2); // 2-4-3
  AddTriangle(pos1 + e1 + e2, pos1, pos2 + e2, col1, col1, col2);      // 214
  AddTriangle(pos1, pos2 + e1, pos2 + e2, col1, col2, col2);           // 134
}

void uiWindow::AddLine(const Vector3& pos1, const Vector3& pos2, const Vector3b& col, const float width) {
  AddLine(pos1, pos2, col, col, width);
}

void uiWindow::AddPoint(const Vector3& pos, const Vector3b& col, const double size) {
  const Vector3 points[] = {
    {pos + Vector3({0.0f, 0.0f, size})},
    {pos + Vector3({size, 0.0f, 0.0f})},
    {pos + Vector3({0.0f, size, 0.0f})},
    {pos + Vector3({size, size, size})},
  };
  AddTriangle(points[0], points[1], points[2], col);
  AddTriangle(points[2], points[1], points[3], col);
  AddTriangle(points[0], points[3], points[1], col);
  AddTriangle(points[2], points[3], points[0], col);
}

void uiWindow::AddPoint_mono_triangle(const Vector3& pos, const Vector3b& col, const double size) {
  AddTriangle(pos + Vector3({size, 0.0f, 0.0f}), pos + Vector3({0.0f, 0.0f, size}), pos + Vector3({0.0f, size, 0.0f}), col);
  AddTriangle(pos + Vector3({0.0f, 0.0f, size}), pos + Vector3({size, 0.0f, 0.0f}), pos + Vector3({0.0f, size, 0.0f}), col);
}

void uiWindow::AddSphere_20(const Vector3& pos, const float size, const Vector3b& col) {
  // 正20面体
  const double t = (1 + sqrt(5)) / 2;
  const Vector3 points[] = {
    pos + Vector3(0.0f, -1.0f, t) * size,  pos + Vector3(0.0f, 1.0f, t) * size,

    pos + Vector3(t, 0.0f, 1.0f) * size,   pos + Vector3(-t, 0.0f, 1.0f) * size,

    pos + Vector3(1.0f, -t, 0.0f) * size,  pos + Vector3(1.0f, t, 0.0f) * size,   pos + Vector3(-1.0f, t, 0.0f) * size, pos + Vector3(-1.0f, -t, 0.0f) * size,

    pos + Vector3(t, 0.0f, -1.0f) * size,  pos + Vector3(-t, 0.0f, -1.0f) * size,

    pos + Vector3(0.0f, -1.0f, -t) * size, pos + Vector3(0.0f, 1.0f, -t) * size,
  };
  float width = -1;
  AddTriangle(points[0], points[2], points[1], col, width);
  AddTriangle(points[1], points[2], points[5], col, width);
  AddTriangle(points[1], points[5], points[6], col, width);
  AddTriangle(points[1], points[6], points[3], col, width);
  AddTriangle(points[1], points[3], points[0], col, width);
  AddTriangle(points[0], points[3], points[7], col, width);
  AddTriangle(points[0], points[7], points[4], col, width);
  AddTriangle(points[0], points[4], points[2], col, width);

  AddTriangle(points[11], points[8], points[10], col, width);
  AddTriangle(points[11], points[5], points[8], col, width);
  AddTriangle(points[11], points[6], points[5], col, width);
  AddTriangle(points[11], points[9], points[6], col, width);
  AddTriangle(points[11], points[10], points[9], col, width);
  AddTriangle(points[9], points[10], points[7], col, width);
  AddTriangle(points[10], points[4], points[7], col, width);
  AddTriangle(points[10], points[8], points[4], col, width);

  AddTriangle(points[5], points[2], points[8], col, width);
  AddTriangle(points[3], points[6], points[9], col, width);
  AddTriangle(points[7], points[3], points[9], col, width);
  AddTriangle(points[2], points[4], points[8], col, width);
}

void uiWindow::AddArrow(const Vector3& pos1, const Vector3& pos2, const Vector3b& col, float width) {
  AddLine(pos1, pos2, col, width);
  const double len = (pos1 - pos2).norm();
  AddSphere_20(pos2, len / 80, col);
}

void uiWindow::AddArrowTo(const Vector3& pos, const Vector3& dir, const Vector3b& col, float width) {
  AddLine(pos, pos + dir, col, width);
  const double len = dir.norm();
  AddSphere_20(pos + dir, len / 80, col);
}

void uiWindow::AddCube(const Vector3& pos, const Vector3& whd, const Vector3b& col) {
  const auto half_size = whd / 2;
  const Vector3 points[8] = {
    pos + Vector3(-half_size[0], -half_size[1], -half_size[2]), pos + Vector3(-half_size[0], -half_size[1], half_size[2]), pos + Vector3(-half_size[0], half_size[1], half_size[2]),
    pos + Vector3(-half_size[0], half_size[1], -half_size[2]),  pos + Vector3(half_size[0], -half_size[1], -half_size[2]), pos + Vector3(half_size[0], -half_size[1], half_size[2]),
    pos + Vector3(half_size[0], half_size[1], half_size[2]),    pos + Vector3(half_size[0], half_size[1], -half_size[2]),
  };
  AddQuad(points[0], points[1], points[2], points[3], col);
  AddQuad(points[3], points[2], points[6], points[7], col);
  AddQuad(points[7], points[6], points[5], points[4], col);
  AddQuad(points[4], points[5], points[1], points[0], col);
  AddQuad(points[0], points[3], points[7], points[4], col);
  AddQuad(points[5], points[6], points[2], points[1], col);
}

void uiWindow::AddCubeLineRotated(const Vector3& pos, const Vector3& size, const Vector3& pry, const Vector3b& col, const float width) {
  const auto half_size = size / 2;
  const Mat3x3 rot0{1, 0, 0, 0, std::cos(pry[0]), -std::sin(pry[0]), 0, std::sin(pry[0]), std::cos(pry[0])};
  const Mat3x3 rot1{std::cos(pry[1]), 0, std::sin(pry[1]), 0, 1, 0, -std::sin(pry[1]), 0, std::cos(pry[1])};
  const Mat3x3 rot2{
    std::cos(pry[2]), -std::sin(pry[2]), 0, std::sin(pry[2]), std::cos(pry[2]), 0, 0, 0, 1,
  };
  const auto rot = rot0 * rot1 * rot2;
  const Vector3 points[8] = {
    pos + rot * Vector3(-half_size[0], -half_size[1], -half_size[2]), pos + rot * Vector3(-half_size[0], -half_size[1], half_size[2]), pos + rot * Vector3(-half_size[0], half_size[1], half_size[2]),
    pos + rot * Vector3(-half_size[0], half_size[1], -half_size[2]),  pos + rot * Vector3(half_size[0], -half_size[1], -half_size[2]), pos + rot * Vector3(half_size[0], -half_size[1], half_size[2]),
    pos + rot * Vector3(half_size[0], half_size[1], half_size[2]),    pos + rot * Vector3(half_size[0], half_size[1], -half_size[2]),
  };
  AddLine(points[0], points[1], col, width);
  AddLine(points[1], points[2], col, width);
  AddLine(points[2], points[3], col, width);
  AddLine(points[3], points[0], col, width);
  AddLine(points[4], points[5], col, width);
  AddLine(points[5], points[6], col, width);
  AddLine(points[6], points[7], col, width);
  AddLine(points[0], points[4], col, width);
  AddLine(points[1], points[5], col, width);
  AddLine(points[2], points[6], col, width);
  AddLine(points[3], points[7], col, width);
}

void uiWindow::AddCubeLine(const Vector3& pos, const Vector3& size, const Vector3b& col, float width) {
  const auto half_size = size / 2;
  const Vector3 points[8] = {
    pos + Vector3(-half_size[0], -half_size[1], -half_size[2]), pos + Vector3(-half_size[0], -half_size[1], half_size[2]), pos + Vector3(-half_size[0], half_size[1], half_size[2]),
    pos + Vector3(-half_size[0], half_size[1], -half_size[2]),  pos + Vector3(half_size[0], -half_size[1], -half_size[2]), pos + Vector3(half_size[0], -half_size[1], half_size[2]),
    pos + Vector3(half_size[0], half_size[1], half_size[2]),    pos + Vector3(half_size[0], half_size[1], -half_size[2]),
  };
  AddLine(points[0], points[1], col, width);
  AddLine(points[1], points[2], col, width);
  AddLine(points[2], points[3], col, width);
  AddLine(points[3], points[0], col, width);
  AddLine(points[4], points[5], col, width);
  AddLine(points[5], points[6], col, width);
  AddLine(points[6], points[7], col, width);
  AddLine(points[0], points[4], col, width);
  AddLine(points[1], points[5], col, width);
  AddLine(points[2], points[6], col, width);
  AddLine(points[3], points[7], col, width);
}
void uiWindow::AddCubeLine(const Vector3& pos, float size, const Vector3b& col, float width) {
  AddCubeLine(pos, Vector3(size, size, size), col, width);
}

void uiWindow::AddPlane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col) {
  Vector3 e1, e2;
  const auto dp = normal.normalize();
  if(normal[0] >= normal[1] && normal[0] >= normal[2]) { // X軸に近い直線
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[2], normal[1], normal[0]};
  } else if(normal[1] >= normal[0] && normal[1] >= normal[2]) {
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[0], normal[2], normal[1]};
  } else {
    e1 = {normal[2], normal[1], normal[0]};
    e2 = {normal[0], normal[2], normal[1]};
  }
  e1 = dp.cross(e1);
  e1 = e1.normalize() / 2.0f;
  e2 = dp.cross(e1);
  e2 = e2.normalize() / 2.0f;
  const Vector3 p[4] = {
    pos + (-e1 - e2) * size[0],
    pos + (e1 - e2) * size[1],
    pos + (e1 + e2) * size[1],
    pos + (-e1 + e2) * size[1],
  };
  AddQuad(p[0], p[1], p[2], p[3], col);
  AddQuad(p[3], p[2], p[1], p[0], col);
}

void uiWindow::AddPlane(const Vector3& pos, const Vector3& size, const Vector3& normal, const Vector3b& col, int width) {
  Vector3 e1, e2;
  const auto dp = normal.normalize();
  if(normal[0] >= normal[1] && normal[0] >= normal[2]) { // X軸に近い直線
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[2], normal[1], normal[0]};
  } else if(normal[1] >= normal[0] && normal[1] >= normal[2]) {
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[0], normal[2], normal[1]};
  } else {
    e1 = {normal[2], normal[1], normal[0]};
    e2 = {normal[0], normal[2], normal[1]};
  }
  e1 = dp.cross(e1);
  e1 = e1.normalize() / 2.0f;
  e2 = dp.cross(e1);
  e2 = e2.normalize() / 2.0f;
  const Vector3 p[4] = {
    pos + (-e1 - e2) * size[0],
    pos + (e1 - e2) * size[1],
    pos + (e1 + e2) * size[1],
    pos + (-e1 + e2) * size[1],
  };
  AddLine(p[0], p[1], col, width);
  AddLine(p[1], p[2], col, width);
  AddLine(p[2], p[3], col, width);
  AddLine(p[3], p[0], col, width);
}

void uiWindow::AddCircle(const Vector3& pos, const Vector3& normal, const Vector3b& col) {
  const auto dp = normal.normalize();
  constexpr int N = 25; // 分割数
  Vector3 e1, e2;
  if(normal[0] >= normal[1] && normal[0] >= normal[2]) { // X軸に近い直線
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[2], normal[1], normal[0]};
  } else if(normal[1] >= normal[0] && normal[1] >= normal[2]) {
    e1 = {normal[1], normal[0], normal[2]};
    e2 = {normal[0], normal[2], normal[1]};
  } else {
    e1 = {normal[2], normal[1], normal[0]};
    e2 = {normal[0], normal[2], normal[1]};
  }

  e1 = dp.cross(e1);
  e1 = e1.normalize();
  e2 = dp.cross(e1);
  e2 = e2.normalize();
  for(int i = 0; i < N + 1; i++) {
    const double theta1 = 6.28 * (float)i / N;
    const double theta2 = 6.28 * (float)(i + 1) / N;
    const auto v1 = (e1 * std::cos(theta1) + e2 * std::sin(theta1));
    const auto v2 = (e1 * std::cos(theta2) + e2 * std::sin(theta2));
    AddTriangle(pos, pos + v2, pos + v1, col);
  }
}

void uiWindow::AddCone(const Vector3& pos, const Vector3& dir, float size, const Vector3b& col) {
  const auto dp = dir.normalize();
  constexpr int N = 25; // 分割数
  Vector3 e1, e2;
  if(dp[0] >= dp[1] && dp[0] >= dp[2]) { // X軸に近い直線
    e1 = {dp[1], dp[0], dp[2]};
    e2 = {dp[2], dp[1], dp[0]};
  } else if(dp[1] >= dp[0] && dp[1] >= dp[2]) {
    e1 = {dp[1], dp[0], dp[2]};
    e2 = {dp[0], dp[2], dp[1]};
  } else {
    e1 = {dp[2], dp[1], dp[0]};
    e2 = {dp[0], dp[2], dp[1]};
  }

  e1 = dp.cross(e1);
  e1 = e1.normalize();
  e2 = dp.cross(e1);
  e2 = e2.normalize();
  for(int i = 0; i < N + 1; i++) {
    const double theta1 = 6.28 * (float)i / N;
    const double theta2 = 6.28 * (float)(i + 1) / N;
    const auto v1 = (e1 * std::cos(theta1) + e2 * std::sin(theta1)) * size;
    const auto v2 = (e1 * std::cos(theta2) + e2 * std::sin(theta2)) * size;
    AddTriangle(pos + dp * size, pos + v2, pos + v1, col);
  }
  AddCircle(pos, dp * size, col);
}


void uiWindow::AddCross(const Vector3& pos, const int n, const Vector3b col) {
  const auto w = std::max<int>(n / 10, 2);
  AddLine(pos - Vector3(n, 0, 0), pos + Vector3(n, 0, 0), col, w);
  AddLine(pos - Vector3(0, n, 0), pos + Vector3(0, n, 0), col, w);
  AddLine(pos - Vector3(0, 0, n), pos + Vector3(0, 0, n), col, w);
}
void uiWindow::AddCoord(const Vector3& pos, const Vector3& axis, const int n) {
  const auto w = std::max<int>(n / 10, 2);
  AddLine(pos, pos + Vector3(n, 0, 0), {0, 0, 255}, w);
  AddLine(pos, pos + Vector3(0, n, 0), {0, 255, 0}, w);
  AddLine(pos, pos + Vector3(0, 0, n), {255, 0, 0}, w);
}

void uiWindow::AddGridXY(const Vector2 range, const int n, const Vector3b col) {
  const int w = std::max<int>(float(range[1] - range[0]) * 0.01, 2);
  for(int i = 0; i < n; i++) {
    const int p = (range[1] - range[0]) * i / n + range[0];
    const auto c = (i == 0 || i == n - 1) ? col : col * 0.8;
    AddLine({(double)p, range[0], 0}, {(double)p, range[1], 0}, c, w);
    AddLine({range[0], (double)p, 0}, {range[1], (double)p, 0}, c, w);
  }
}

void uiWindow::AddRotatedRectPosSize(const Vector2d& pos, const Vector2d& size, const double theta, const Vector3b& col) {
  const Mat2x2 rmat = {std::cos(theta), -std::sin(theta), std::sin(theta), std::cos(theta)};
  const auto hs = size / 2;
  AddQuad2D(pos + rmat * Vector2(hs[0], hs[1]), pos + rmat * Vector2(-hs[0], hs[1]), pos + rmat * Vector2(-hs[0], -hs[1]), pos + rmat * Vector2(hs[0], -hs[1]), col);
}

void uiWindow::AddRotatedRectPosSize(const Vector2d& pos, const Vector2d& size, const double theta, const Vector3b& col, const float width) {
  const Mat2x2 rmat = {std::cos(theta), -std::sin(theta), std::sin(theta), std::cos(theta)};
  const auto hs = size / 2;
  AddQuad2D(pos + rmat * Vector2(hs[0], hs[1]), pos + rmat * Vector2(-hs[0], hs[1]), pos + rmat * Vector2(-hs[0], -hs[1]), pos + rmat * Vector2(hs[0], -hs[1]), col, width);
}

void uiWindow::AddArrow2D(const Vector2d& from, const Vector2d& to, const Vector3b& col, const float width) {
  AddLine2D(from, to, col, width);
  const auto d = to - from;
  const double theta = std::atan2(d[1], d[0]);
  constexpr double dt = 0.5;
  constexpr int l = 50;
  const auto pa = Vector2d(std::cos(theta + dt), std::sin(theta + dt)) * l;
  const auto pb = Vector2d(std::cos(theta - dt), std::sin(theta - dt)) * l;
  AddLine2D(to - pa, to, col, width);
  AddLine2D(to - pb, to, col, width);
}

Vector2d uiWindow::get_text_size(const std::string& str, float size) const {
  int x = 0;
  int y = 0;
  const std::u32string u32str = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(str);
  const auto text_renderer = ::vkUI::Engine::getTextRendererPtr();
  const int spacing = getStyle()->TextSpacing;
  Vector2d whole_size{0, 0};
  if(u32str.size() == 0) return {0, 0};
  y += text_renderer->FindGlyph(u32str[0])->dHeight * size;
  for(int i = 0; i < u32str.size(); i++) {
    if((uiWchar)u32str[i] == '\n' && u32str.size() > i + 1) {
      x = 0;
      y = whole_size[1] + text_renderer->FindGlyph(u32str[i + 1])->dHeight * size + spacing;
      continue;
    }
    const auto glyph = text_renderer->FindGlyph(u32str[i]);
    const int y_tmp = y + (-glyph->dHeight + glyph->V1 - glyph->V0) * size;
    x += spacing + (glyph->U1 - glyph->U0) * size;
    whole_size = {std::max<int>(whole_size[0], x), std::max<int>(whole_size[1], y_tmp)};
  }
  return whole_size;
}

void uiWindow::AddStringBalloon(const std::string& str, const Vector3& pos, const float size, const Vector3b& col, const Vector3b& line) {
  /* _Mat<float, 4, 4> R1; */
  /* camera_position.getCameraProjMatrix(R1.value); */
  /* const _Mat<float, 4, 4> R2{ */
  /*   float(this->size[0])/2.0f, 0.0f, 0.0f, 0.0f, */
  /*   0.0f, float(this->size[1])/2.0f, 0.0f, 0.0f, */
  /*   0.0f, 0.0f, 1.0f, 0.0f, */
  /*   -1.0f, -1.0f, 0.0f, 1.0f */
  /* }; */
  /* auto projected = R1 * Vector4{pos[0], pos[1], pos[2], 1.0 }; */
  float R1[16];
  camera_position.getCameraProjMatrix(R1);
  float projected[4];
  projected[0] = R1[0] * pos[0] + R1[1] * pos[1] + R1[2] * pos[2] + R1[3] * 1;
  projected[1] = R1[4] * pos[0] + R1[5] * pos[1] + R1[6] * pos[2] + R1[7] * 1;
  projected[3] = R1[12] * pos[0] + R1[13] * pos[1] + R1[14] * pos[2] + R1[15] * 1;

  const auto to = Vector2d{(int)(projected[0] * this->size[0] / 2.0f / projected[3] + this->size[0] / 2.0f), (int)(projected[1] * this->size[1] / 2.0f / projected[3] + this->size[1] / 2.0f)};
  AddStringBalloon2D(str, to, size, col, line);
}


void uiWindow::AddStringBalloon2D(const std::string& str, const Vector2d& to, const float size, const Vector3b& col, const Vector3b& line) {
  const bool is_arrow_up = to[1] > 35; // 通常True
  const auto tsize = get_text_size(str, size);
  int x, y;
  constexpr int line_width = 2;
  constexpr int leader_len = 25;
  if(is_arrow_up) {
    AddLine2D(to, to + Vector2d(3, -5), line, line_width);
    AddLine2D(to, to + Vector2d(leader_len, -leader_len), line, line_width);
    AddLine2D(to + Vector2d(leader_len, -leader_len), to + Vector2d(leader_len + tsize[0] + 10, -leader_len), line, line_width);
    x = to[0] + leader_len + 5;
    y = to[1] - leader_len - tsize[1] - 10;
  } else {
    AddLine2D(to, to + Vector2d(3, 5), line, line_width);
    AddLine2D(to, to + Vector2d(leader_len, leader_len), line, line_width);
    AddLine2D(to + Vector2d(leader_len, leader_len), to + Vector2d(leader_len + tsize[0] + 10, leader_len), line, line_width);
    x = to[0] + leader_len + 5;
    y = to[1] + leader_len - tsize[1] - 10;
  }
  AddString2D(str, {x, y}, 1, col);
}

// ----------------------------------------------------------------
//     [ SECTION ] 2D Drawing Functions
// ----------------------------------------------------------------
void uiWindow::__AddPointSizeZero2D(const Vector2d& pos, const Vector3b& col) {
  const auto text_renderer = getTextRendererPtr();
  dd.vertices_ui.push_back(std::move(VertexUI(pos, col, text_renderer->TexUvWhitePixel)));
}

Vector2d uiWindow::AddString2D(const std::string& str, const Vector2d& pos, const float size, const Vector3b& col, const int xlim) {
  const std::u32string u32str = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(str);
  int x = pos[0];
  int y = pos[1];
  const auto text_renderer = ::vkUI::Engine::getTextRendererPtr();
  const int spacing = getStyle()->TextSpacing;
  Vector2d whole_size{0, 0};
  // const int w = text_renderer->TexWidth;
  // const int h = text_renderer->TexHeight;
  if(u32str.size() == 0) {
    return {0, 0};
  }

  y += text_renderer->FindGlyph(u32str[0])->dHeight * size;
  for(int i = 0; i < u32str.size(); i++) {
    const auto glyph = text_renderer->FindGlyph(u32str[i]);
    const int x2 = x + (glyph->U1 - glyph->U0) * size;
    if(((uiWchar)u32str[i] == '\n' || x2 - pos[0] > xlim) && u32str.size() > i + 1) {
      x = pos[0];
      y = pos[1] + whole_size[1] + text_renderer->FindGlyph(u32str[i + 1])->dHeight * size + spacing;
      continue;
    }
    const int y_tmp = y + (-glyph->dHeight + glyph->V1 - glyph->V0) * size;
    const int y1 = std::max<int>(0, y - glyph->dHeight * size);
    const int y2 = std::max<int>(0, y_tmp);
    AddQuad2D({x, y1}, {x, y2}, {x2, y2}, {x2, y1}, {glyph->U0, glyph->V0}, {glyph->U0, glyph->V1}, {glyph->U1, glyph->V1}, {glyph->U1, glyph->V0}, col);
    x += spacing + (glyph->U1 - glyph->U0) * size;

    whole_size = {std::max(whole_size[0], x - pos[0]), std::max(whole_size[1], y_tmp - pos[1])};
  }
  return whole_size;
}

Vector2d uiWindow::AddString2D(const std::string& str, const Vector2d& pos, const float size, const uiRect& clip_rect, const Vector3b& col) {
  const std::u32string u32str = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(str);
  int x = pos[0];
  int y = pos[1];
  const auto text_renderer = ::vkUI::Engine::getTextRendererPtr();
  const int spacing = getStyle()->TextSpacing;
  Vector2d whole_size{0, 0};
  // const int w = text_renderer->TexWidth;
  // const int h = text_renderer->TexHeight;
  if(u32str.size() == 0) {
    return {0, 0};
  }

  y += text_renderer->FindGlyph(u32str[0])->dHeight * size;
  for(int i = 0; i < u32str.size(); i++) {
    if((uiWchar)u32str[i] == '\n' && u32str.size() > i + 1) {
      x = pos[0];
      y = pos[1] + whole_size[1] + text_renderer->FindGlyph(u32str[i + 1])->dHeight * size + spacing;
      continue;
    }
    const auto glyph = text_renderer->FindGlyph(u32str[i]);
    const int y_tmp = y + (-glyph->dHeight + glyph->V1 - glyph->V0) * size;
    const int y1 = std::max<int>(0, y - glyph->dHeight * size);
    const int y2 = std::max<int>(0, y_tmp);
    const int x2 = x + (glyph->U1 - glyph->U0) * size;
    if(clip_rect.isContains(x, y1) && clip_rect.isContains(x2, y2)) {
      AddQuad2D({x, y1}, {x, y2}, {x2, y2}, {x2, y1}, {glyph->U0, glyph->V0}, {glyph->U0, glyph->V1}, {glyph->U1, glyph->V1}, {glyph->U1, glyph->V0}, col);
    }
    x += spacing + (glyph->U1 - glyph->U0) * size;

    whole_size = {std::max(whole_size[0], x - pos[0]), std::max(whole_size[1], y_tmp - pos[1])};
  }
  return whole_size;
}

void uiWindow::terminate() {
  glfwDestroyWindow(window);
  glfwSwapBuffers(window);
  glfwPollEvents();
}

} // namespace vkUI::Engine
