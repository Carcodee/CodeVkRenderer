﻿//


// Created by carlo on 2024-09-24.
//







#ifndef PRESENTQUEUE_HPP
#define PRESENTQUEUE_HPP

namespace ENGINE
{
    struct PresentQueue
    {
    public:

        PresentQueue(Core* core, WindowDesc windowDesc, uint32_t imagesCount, vk::PresentModeKHR preferredMode, glm::uvec2 windowSize)
        {
            this->core = core;
            this->swapChain = core->CreateSwapchain(preferredMode, imagesCount, windowDesc,  windowSize);
            this->swapchainImageViews = swapChain->GetImageViews();
            this->swapchainRect = vk::Rect2D(vk::Offset2D(), swapChain->extent);
            this->imageIndex = -1;
            
        }
        ImageView* AcquireImage(vk::Semaphore signalSemaphore)
        {
            this->imageIndex = swapChain->AcquireNextImage(signalSemaphore).value;
            return swapchainImageViews[imageIndex];
        }
        void PresentImage(vk::Semaphore waitSemaphore)
        {
            vk::SwapchainKHR swapchains[] = {swapChain.get()->swapchainHandle.get()};
            vk::Semaphore waitSemaphores[] = {waitSemaphore};
            
            auto presentInfo = vk::PresentInfoKHR()
            .setSwapchainCount(1)
            .setPSwapchains(swapchains)
            .setPImageIndices(&imageIndex)
            .setPResults(nullptr)
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(waitSemaphores);

            auto res = core->presentQueue.presentKHR(presentInfo);
            
        }
        
        vk::Extent2D GetImageSize()
        {
            return swapChain.get()->extent;
        }
        Core* core;
        std::unique_ptr<SwapChain> swapChain;
        std::vector<ImageView*> swapchainImageViews;
        uint32_t imageIndex;
        vk::Rect2D swapchainRect;
        
    };

    struct InFlightQueue
    {
        InFlightQueue(Core* core, WindowDesc windowDesc, uint32_t inflightCount, vk::PresentModeKHR preferredMode, glm::uvec2 windowSize)
        {
            
            this->core = core;
            presentQueue.reset(new PresentQueue(this->core, windowDesc, inflightCount, preferredMode, windowSize));

            for (int frameIndex = 0; frameIndex < inflightCount; frameIndex++)
            {
                FrameResources frame;
                frame.inflightFence = core->CreateFence(true);
                frame.imageAcquiredSemaphore = core->CreateVulkanSemaphore();
                frame.renderingFinishedSemaphore = core->CreateVulkanSemaphore();
                frame.commandBuffer= std::move(core->AllocateCommandBuffers(1)[0]);
                frameResources.push_back(std::move(frame)); 
            }
            frameIndex = 0;
            
        }
        void BeginFrame()
        {

            auto &currFrame = frameResources[frameIndex];
            {
                core->WaitForFence(currFrame.inflightFence.get());
                core->ResetFence(currFrame.inflightFence.get());
            }

            {
                currentSwapchainImageView = presentQueue->AcquireImage(currFrame.imageAcquiredSemaphore.get());
            }

            auto bufferBeginInfo = vk::CommandBufferBeginInfo()
                .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
            currFrame.commandBuffer->begin(bufferBeginInfo);

            //add pass info from my data
        }
        void EndFrame()
        {
            auto &currFrame = frameResources[frameIndex];


            ImageAccessPattern pattern = GetImageDstPattern(PRESENT);
            TransitionImage(currentSwapchainImageView->imageData, pattern, currentSwapchainImageView->GetSubresourceRange(),
                                    currFrame.commandBuffer.get());
            
            currFrame.commandBuffer->end();
            {
                vk::Semaphore waitSemaphores[] = {currFrame.imageAcquiredSemaphore.get()};
                vk::Semaphore signalSemaphores[] = {currFrame.renderingFinishedSemaphore.get()};
                vk::PipelineStageFlags waitStages [] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

                auto submitInfo = vk::SubmitInfo()
                .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(waitSemaphores)
                .setPWaitDstStageMask(waitStages)
                .setCommandBufferCount(1)
                .setPCommandBuffers(&currFrame.commandBuffer.get())
                .setSignalSemaphoreCount(1)
                .setPSignalSemaphores(signalSemaphores);

                core->presentQueue.submit({submitInfo}, currFrame.inflightFence.get());
            }
            presentQueue->PresentImage(currFrame.renderingFinishedSemaphore.get());
            frameIndex = (frameIndex + 1) % frameResources.size();
            
        }

       
        std::vector<FrameResources> frameResources;
        size_t frameIndex;
        
        Core* core;
        std::unique_ptr<PresentQueue> presentQueue;
        ImageView* currentSwapchainImageView;
    };
    class ExecuteOnceCommand
    {
    public:
        ExecuteOnceCommand(Core* core)
        {
            this->core = core;
            commandBufferHandle = std::move(core->AllocateCommandBuffers(1)[0]);
        }

        vk::CommandBuffer BeginCommandBuffer()
        {
            auto bufferBeginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            commandBufferHandle->begin(bufferBeginInfo);
            return commandBufferHandle.get();
        }

        void EndCommandBuffer()
        {
            commandBufferHandle->end();
            vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eAllCommands};

            auto submitInfo = vk::SubmitInfo()
            .setWaitSemaphoreCount(0)
            .setPWaitDstStageMask(waitStages)
            .setCommandBufferCount(1)
            .setPCommandBuffers(&commandBufferHandle.get())
            .setSignalSemaphoreCount(0);

            core->graphicsQueue.submit({submitInfo}, nullptr);
            core->graphicsQueue.waitIdle();
            
        }
        Core* core;
        vk::UniqueCommandBuffer commandBufferHandle;
        
    };
}

#endif //PRESENTQUEUE_HPP
