﻿//
// Created by carlo on 2024-09-21.
//

#include "VulkanAPI/EngineInclude.hpp"

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_RIGHT_HANDED
#define GLM_ENABLE_EXPERIMENTAL

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

CONST int WINDOWS_WIDTH = 1024;
CONST int WINDOWS_HEIGHT = 1024;

glm::uvec2 GetWindowSize(GLFWwindow* window)
{
    int widht = 0;
    int height = 0;

    glfwGetWindowSize(window, &widht,&height);

    return glm::uvec2(widht, height);
    
}
void run()
{
    GLFWwindow* window = glfwCreateWindow(WINDOWS_WIDTH, WINDOWS_HEIGHT, "Vulkan Engine Template", nullptr, nullptr);
    {
        int imageCount = 3;

        const char* glfwExtensions[] = {"VK_KHR_surface", "VK_KHR_win32_surface"};
        uint32_t glfwExtensionCount = sizeof(glfwExtensions) / sizeof(glfwExtensions[0]);

        ENGINE::WindowDesc windowDesc = {};
        windowDesc.hInstance = GetModuleHandle(NULL);
        windowDesc.hWnd = glfwGetWin32Window(window);

        bool enableDebugging = true;
#if defined ENGINE_ENABLE_DEBUGGING
    enableDebugging = true;
#endif

        
        std::unique_ptr<ENGINE::Core> core = std::make_unique<ENGINE::Core>(
            glfwExtensions, glfwExtensionCount, &windowDesc, enableDebugging);
        std::unique_ptr<ENGINE::InFlightQueue> inFlightQueue = std::make_unique<ENGINE::InFlightQueue>(
            core.get(), windowDesc, imageCount, vk::PresentModeKHR::eMailbox,
            GetWindowSize(window));
        // std::unique_ptr<ENGINE::ExecuteOnceCommand> executeOnceCommand =std::make_unique<ENGINE::ExecuteOnceCommand>(core.get());

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            {
                if (!inFlightQueue)
                {
                    std::cout << "recreated swapchain";
                    inFlightQueue = std::make_unique<ENGINE::InFlightQueue>(
                        core.get(), windowDesc, imageCount, vk::PresentModeKHR::eMailbox,
                        GetWindowSize(window));
                }
                try
                {
                    inFlightQueue->BeginFrame();
                    
                    inFlightQueue->EndFrame();    
                }catch (vk::OutOfDateKHRError err)
                {
                    core->WaitIdle();
                    inFlightQueue.reset();
                }
                
                
            }
            core->WaitIdle();
        }
        glfwDestroyWindow(window);
    }
}

int main()
{
    glfwInit();

    run();

    glfwTerminate();

    return 0;
}
