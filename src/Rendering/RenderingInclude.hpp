﻿//

// Created by carlo on 2024-10-07.
//



#pragma once


#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include <imgui.h>


#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "RendererStructs.hpp"

#include "RThings/Model.hpp"
#include "RThings/ModelLoader.hpp"
#include "RThings/Camera.hpp"


#include "BaseRenderer.hpp"
#include "Examples/ForwardRenderer.hpp"
#include "Examples/ComputeRenderer.hpp"
#include "ImguiRenderer.hpp"


