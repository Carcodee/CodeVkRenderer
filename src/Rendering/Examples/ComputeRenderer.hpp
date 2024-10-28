﻿//

// Created by carlo on 2024-10-08.
//



#ifndef COMPUTERENDERER_HPP
#define COMPUTERENDERER_HPP

namespace Rendering
{
    class ComputeRenderer : BaseRenderer
    {
    public:
        ComputeRenderer(ENGINE::Core* core, WindowProvider* windowProvider,
                        ENGINE::DescriptorAllocator* descriptorAllocator)
        {
            this->core = core;
            this->renderGraphRef = core->renderGraphRef;
            this->windowProvider = windowProvider;
            this->descriptorAllocatorRef = descriptorAllocator;
            auto logicalDevice = core->logicalDevice.get();
            auto physicalDevice = core->physicalDevice;

            auto imageInfo = ENGINE::Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
                                                         renderGraphRef->storageImageFormat,
                                                         vk::ImageUsageFlagBits::eStorage);
            
            computeImage = std::make_unique<ENGINE::Image>(physicalDevice, logicalDevice, imageInfo);
            computeImageData = std::make_unique<ENGINE::ImageData>(computeImage->imageHandle.get(), vk::ImageType::e2D,
                                                                   glm::vec3(windowProvider->GetWindowSize().x,
                                                                             windowProvider->GetWindowSize().y, 1), 1,
                                                                   1,
                                                                   renderGraphRef->storageImageFormat,
                                                                   vk::ImageLayout::eUndefined);
            computeImageView = std::make_unique<ENGINE::ImageView>(logicalDevice, computeImageData.get(), 0, 1, 0, 1);
            
            ENGINE::Sampler* computeImageSampler = renderGraphRef->samplerPool.GetSampler(
                vk::SamplerAddressMode::eRepeat, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);

            compShader = std::make_unique<ENGINE::Shader>(logicalDevice,"C:\\Users\\carlo\\CLionProjects\\Vulkan_Engine_Template\\src\\Shaders\\spirv\\Examples\\cSample.comp.spv");
            
            ENGINE::DescriptorLayoutBuilder builder;

            ENGINE::ShaderParser::GetLayout(*compShader->sParser.get(), builder);

            dstLayout= builder.BuildBindings(logicalDevice, vk::ShaderStageFlagBits::eCompute);

            auto layoutCreateInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(1)
            .setPSetLayouts(&dstLayout.get());
            
            dstSet = descriptorAllocatorRef->Allocate(core->logicalDevice.get(), dstLayout.get());
            
            writerBuilder.AddWriteImage(0, computeImageView.get(), computeImageSampler->samplerHandle.get(),
                                        vk::ImageLayout::eGeneral,
                                        vk::DescriptorType::eStorageImage);
            writerBuilder.UpdateSet(core->logicalDevice.get(), dstSet.get());
            
            computeNodeName = "compute";
            auto renderNode = renderGraphRef->AddPass(computeNodeName);
            
            renderNode->SetCompShader(compShader.get());
            renderNode->SetPipelineLayoutCI(layoutCreateInfo);
            renderNode->AddNodeStorageImg("storageImage", computeImageView.get());
            renderNode->BuildRenderGraphNode();
        }
        void RecreateSwapChainResources() override
        {
            
        }
        void SetRenderOperation(ENGINE::InFlightQueue* inflightQueue) override
        {
            auto renderOp = new std::function<void(vk::CommandBuffer& command_buffer)>(
                [this](vk::CommandBuffer& commandBuffer)
                {

                    auto& renderNode = renderGraphRef->renderNodes.at(computeNodeName);
                    commandBuffer.bindDescriptorSets(renderNode->pipelineType,
                                                     renderNode->pipelineLayout.get(), 0,
                                                     1,
                                                     &dstSet.get(), 0, nullptr);
                    commandBuffer.bindPipeline(renderNode->pipelineType, renderNode->pipeline.get());
                    commandBuffer.dispatch(windowProvider->GetWindowSize().x, windowProvider->GetWindowSize().y, 1);
                });
            
            renderGraphRef->GetNode(computeNodeName)->SetRenderOperation(renderOp);
        }

        void ReloadShaders() override
        {
            auto renderNode = renderGraphRef->GetNode(computeNodeName);
            renderNode->RecreateResources();
        }
        
        ENGINE::DescriptorAllocator* descriptorAllocatorRef;
        WindowProvider* windowProvider;
        ENGINE::Core* core;
        ENGINE::RenderGraph* renderGraphRef;
        
        ENGINE::DescriptorWriterBuilder writerBuilder;
        vk::UniqueDescriptorSetLayout dstLayout;
        vk::UniqueDescriptorSet dstSet;
        std::unique_ptr<ENGINE::Shader> compShader;

        std::string computeNodeName;
        
        std::unique_ptr<ENGINE::Image> computeImage;
        std::unique_ptr<ENGINE::ImageData> computeImageData;
        std::unique_ptr<ENGINE::ImageView> computeImageView;
 
    };
}

#endif //COMPUTERENDERER_HPP
