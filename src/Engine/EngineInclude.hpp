﻿//
// Created by carlo on 2024-09-22.
//

#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX

#include <cstdint>
#include <memory>
#include <iostream>
#include <algorithm>
#include <list>
#include <fstream>
#include <map>
#include<functional>
#include <set>
#include<deque>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>

#include <spirv_glsl.hpp>

#include <vulkan/vulkan.hpp>
#include "Structs.hpp"
#include "UtilVk.hpp"
#include "Buffer.hpp"
#include "StagedBuffer.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "Sampler.hpp"
#include "SyncronizationPatterns.hpp"
#include "DescriptorAllocator.hpp"
#include "Descriptors.hpp"
#include "ShaderModule.hpp"
#include "ShaderParser.hpp"
#include "VertexInput.hpp"
#include "Surface.hpp"
#include "SwapChain.hpp"
#include "DynamicRenderPass.hpp"
#include "Pipeline.hpp"

#include "Core.hpp"
#include "RenderGraph.hpp"
#include "CoreImpl.hpp"

#include "PresentQueue.hpp"

#include "ImageShipper.hpp"
#include "ResourcesManager.hpp"
#include "DescriptorCache.hpp"

