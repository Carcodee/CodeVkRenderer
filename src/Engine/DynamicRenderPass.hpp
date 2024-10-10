﻿//
// Created by carlo on 2024-09-30.
//

#ifndef DYNAMICRENDERPASS_HPP
#define DYNAMICRENDERPASS_HPP

namespace ENGINE
{

    //TODO: set base configs 

    struct AttachmentInfo
    {
        vk::RenderingAttachmentInfo attachmentInfo;
        vk::Format format;
    };

    static AttachmentInfo GetColorAttachmentInfo(vk::Format format = vk::Format::eB8G8R8A8Srgb,
                                                 vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
                                                 vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore)
    {
        AttachmentInfo attachmentInfo;
        attachmentInfo.attachmentInfo = vk::RenderingAttachmentInfo()
                                        .setImageView(nullptr)
                                        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                        .setLoadOp(loadOp)
                                        .setStoreOp(storeOp)
                                        .setClearValue(
                                            vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{
                                                0.0f, 0.0f, 0.0f, 1.0f
                                            })));

        attachmentInfo.format = format;
        
        return attachmentInfo;
    }

    static AttachmentInfo GetDepthAttachmentInfo(vk::Format format = vk::Format::eD32Sfloat)
    {
        AttachmentInfo attachmentInfo;
        attachmentInfo.attachmentInfo = vk::RenderingAttachmentInfo()
                               .setImageView(nullptr)
                               .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
                               .setLoadOp(vk::AttachmentLoadOp::eClear)
                               .setStoreOp(vk::AttachmentStoreOp::eStore)
                               .setClearValue(vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)));
        attachmentInfo.format = format;
        
        return attachmentInfo;
    }


    struct DynamicRenderPass
    {
    public:
        void SetPipelineRenderingInfo(uint32_t colorAttachmentCount,
                          std::vector<vk::Format>& colorFormats,
                          vk::Format depthFormat = vk::Format::eD32Sfloat)
        {
            if (colorFormats.empty())
            {
                colorFormats.push_back(vk::Format::eB8G8R8A8Srgb);
            }
            this->colorFormat = colorFormat;
            this->depthFormat = depthFormat;
            this->expectedColorAttachmentSize = colorAttachmentCount;
            pipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo()
            .setColorAttachmentCount(colorAttachmentCount)
            .setPColorAttachmentFormats(colorFormats.data())
            .setDepthAttachmentFormat(this->depthFormat)
            .setStencilAttachmentFormat(vk::Format::eUndefined);
            
        }
        void SetRenderInfo(std::vector<vk::RenderingAttachmentInfo>& colorAttachments,
                           glm::uvec2 framebufferSize, vk::RenderingAttachmentInfo* depthAttachment)
        {
            for (auto& element : colorAttachments)
            {
                assert(element.imageView != nullptr && "Image view was not set before setting the render info");
            }
            assert(
                colorAttachments.size() == expectedColorAttachmentSize &&
                "Color attachment must be the same as the one indicated in the pipeline creation");
             renderInfo = vk::RenderingInfo()
            .setRenderArea({{0, 0},{framebufferSize.x, framebufferSize.y}})
            .setLayerCount(1)
            .setColorAttachmentCount(colorAttachments.size())
            .setPColorAttachments(colorAttachments.data());
            if (depthAttachment->imageView != nullptr)
            {
                renderInfo.setPDepthAttachment(depthAttachment);
            }
        }

        void SetRenderInfoUnsafe(std::vector<vk::RenderingAttachmentInfo>& colorAttachments,
                           glm::uvec2 framebufferSize, vk::RenderingAttachmentInfo* depthAttachment)
        {
            for (auto& element : colorAttachments)
            {
                assert(element.imageView != nullptr && "Image view was not set before setting the render info");
            }
             renderInfo = vk::RenderingInfo()
            .setRenderArea({{0, 0},{framebufferSize.x, framebufferSize.y}})
            .setLayerCount(1)
            .setColorAttachmentCount(colorAttachments.size())
            .setPColorAttachments(colorAttachments.data());
            if (depthAttachment->imageView != nullptr)
            {
                renderInfo.setPDepthAttachment(depthAttachment);
            }
        }
        void SetViewport(glm::uvec2 viewportSize, glm::uvec2 scissorSize)
        {
            viewport
                .setX(0.0f)
                .setY(0.0f)
                .setWidth(static_cast<float>(viewportSize.x))
                .setHeight(static_cast<float>(viewportSize.y))
                .setMinDepth(0.0f)
                .setMaxDepth(1.0f);
            scissor
                .setOffset({0, 0})
                .setExtent({scissorSize.x, scissorSize.y});
            
        }
        vk::Format colorFormat;
        vk::Format depthFormat;
        uint32_t expectedColorAttachmentSize = 0;
        vk::RenderingInfo renderInfo;
        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo;
        vk::Viewport viewport;
        vk::Rect2D scissor;
        
    };
}
#endif //DYNAMICRENDERPASS_HPP
