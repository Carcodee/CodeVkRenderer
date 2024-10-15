﻿//
// Created by carlo on 2024-10-02.
//






#ifndef RENDERGRAPH_HPP
#define RENDERGRAPH_HPP


namespace ENGINE
{

    class RenderGraph;
    struct RenderGraphNode
    {


        RenderGraphNode(){
        }

        void RecreateResources()
        {
             assert(&pipelineLayoutCI != nullptr && "Pipeline layout is null");
            pipeline.reset();

            if (fragShaderModule && vertShaderModule)
            {
                std::vector<vk::Format> colorFormats;
                colorFormats.reserve(colAttachments.size());
                std::vector<vk::RenderingAttachmentInfo> renderingAttachmentInfos;
                for (auto& colAttachment : colAttachments)
                {
                    colorFormats.push_back(colAttachment.format);
                    renderingAttachmentInfos.push_back(colAttachment.attachmentInfo);
                }
                dynamicRenderPass.SetPipelineRenderingInfo(colAttachments.size(), colorFormats, depthAttachment.format);
            
                pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
                std::unique_ptr<GraphicsPipeline> graphicsPipeline = std::make_unique<ENGINE::GraphicsPipeline>(
                    core->logicalDevice.get(), vertShaderModule->shaderModuleHandle.get(),
                    fragShaderModule->shaderModuleHandle.get(), pipelineLayout.get(),
                    dynamicRenderPass.pipelineRenderingCreateInfo,
                    colorBlendConfigs, depthConfig,
                    vertexInput, pipelineCache.get()
                );
                pipeline = std::move(graphicsPipeline->pipelineHandle);
                pipelineType = vk::PipelineBindPoint::eGraphics;
                std::cout << "Graphics pipeline created\n";
                
            }else if(compShaderModule)
            {
                pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
                std::unique_ptr<ComputePipeline> computePipeline;
                std::unique_ptr<ComputePipeline> graphicsPipeline = std::make_unique<ENGINE::ComputePipeline>(
                    core->logicalDevice.get(), compShaderModule->shaderModuleHandle.get(), pipelineLayout.get(),
                    pipelineCache.get());
                pipeline = std::move(computePipeline->pipelineHandle);
                pipelineType = vk::PipelineBindPoint::eCompute;
                std::cout << "Compute pipeline created\n";
            }else
            {
                std::cout << "No compute or graphics shaders were set\n";
            }
                       
        }
        void BuildRenderGraphNode()
        {
            assert(&pipelineLayoutCI != nullptr && "Pipeline layout is null");
            auto pipelineCacheCreateInfo = vk::PipelineCacheCreateInfo();
            pipelineCache = core->logicalDevice->createPipelineCacheUnique(pipelineCacheCreateInfo);
 
            if (fragShaderModule && vertShaderModule)
            {
                std::vector<vk::Format> colorFormats;
                colorFormats.reserve(colAttachments.size());
                std::vector<vk::RenderingAttachmentInfo> renderingAttachmentInfos;
                for (auto& colAttachment : colAttachments)
                {
                    colorFormats.push_back(colAttachment.format);
                    renderingAttachmentInfos.push_back(colAttachment.attachmentInfo);
                }
                dynamicRenderPass.SetPipelineRenderingInfo(colAttachments.size(), colorFormats, depthAttachment.format);
            
                pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);

               
                std::unique_ptr<GraphicsPipeline> graphicsPipeline = std::make_unique<ENGINE::GraphicsPipeline>(
                    core->logicalDevice.get(), vertShaderModule->shaderModuleHandle.get(),
                    fragShaderModule->shaderModuleHandle.get(), pipelineLayout.get(),
                    dynamicRenderPass.pipelineRenderingCreateInfo,
                    colorBlendConfigs, depthConfig,
                    vertexInput, pipelineCache.get()
                );
                pipeline = std::move(graphicsPipeline->pipelineHandle);
                pipelineType = vk::PipelineBindPoint::eGraphics;
                std::cout << "Graphics pipeline created\n";
                
            }else if(compShaderModule)
            {
                pipelineLayout = core->logicalDevice->createPipelineLayoutUnique(pipelineLayoutCI);
                std::unique_ptr<ComputePipeline> computePipeline;
                std::unique_ptr<ComputePipeline> graphicsPipeline = std::make_unique<ENGINE::ComputePipeline>(
                    core->logicalDevice.get(), compShaderModule->shaderModuleHandle.get(), pipelineLayout.get(), 
                    pipelineCache.get());
                pipeline = std::move(computePipeline->pipelineHandle);
                pipelineType = vk::PipelineBindPoint::eCompute;
                std::cout << "Compute pipeline created\n";
            }else
            {
                std::cout << "No compute or graphics shaders were set\n";
            }
            
        }
        void TransitionImages(vk::CommandBuffer commandBuffer)
        {
            for (auto& storageImage : storageImages)
            {
                LayoutPatterns dstPattern = EMPTY;
                switch (pipelineType)
                {
                case vk::PipelineBindPoint::eGraphics:
                    dstPattern = GRAPHICS_WRITE;
                    break;
                case vk::PipelineBindPoint::eCompute:
                    dstPattern = COMPUTE_WRITE;
                    break;
                default:
                    assert(false && "pipeline type is unknown");
                }
                if (IsImageTransitionNeeded(storageImage.second->imageData->currentLayout, dstPattern))
                {
                    TransitionImage(storageImage.second->imageData, dstPattern,
                                    storageImage.second->GetSubresourceRange(),
                                    commandBuffer);
                }
            }
            for (auto& sampler : sampledImages)
            {
                LayoutPatterns dstPattern = EMPTY;
                switch (pipelineType)
                {
                case vk::PipelineBindPoint::eGraphics:
                    dstPattern = GRAPHICS_READ;
                    break;
                case vk::PipelineBindPoint::eCompute:
                    dstPattern = GRAPHICS_READ;
                    break;
                default:
                    assert(false && "pipeline type is unknown");
                }
                if (IsImageTransitionNeeded(sampler.second->imageData->currentLayout, dstPattern))
                {
                    TransitionImage(sampler.second->imageData, dstPattern, sampler.second->GetSubresourceRange(),
                                    commandBuffer);
                }
            }           
        }
        void ExecutePass(vk::CommandBuffer commandBuffer)
        {
 
            dynamicRenderPass.SetViewport(frameBufferSize, frameBufferSize);
            commandBuffer.setViewport(0,1,&dynamicRenderPass.viewport);
            commandBuffer.setScissor(0, 1, &dynamicRenderPass.scissor);
            
            assert(imagesAttachment.size()== colAttachments.size()&& "Not all color attachments were set");
            int index = 0;
            std::vector<vk::RenderingAttachmentInfo> attachmentInfos;
            attachmentInfos.reserve(colAttachments.size());
            for (auto& imagePair : imagesAttachment)
            {
                if (IsImageTransitionNeeded(imagePair.second->imageData->currentLayout, COLOR_ATTACHMENT))
                {
                    TransitionImage(imagePair.second->imageData, COLOR_ATTACHMENT, imagePair.second->GetSubresourceRange(), commandBuffer);
                }
                colAttachments[index].attachmentInfo.setImageView(imagePair.second->imageView.get());
                attachmentInfos.push_back(colAttachments[index].attachmentInfo);
                index++;
            }

            TransitionImages(commandBuffer);
                       
            if (depthImage != nullptr)
            {
                depthAttachment.attachmentInfo.imageView = depthImage->imageView.get();
            }
            dynamicRenderPass.SetRenderInfo(attachmentInfos, frameBufferSize, &depthAttachment.attachmentInfo);
            commandBuffer.bindPipeline(pipelineType, pipeline.get());
            commandBuffer.beginRendering(dynamicRenderPass.renderInfo);
            (*renderOperations)(commandBuffer);
            commandBuffer.endRendering();           
        }
        void ExecuteCompute(vk::CommandBuffer commandBuffer)
        {
            TransitionImages(commandBuffer);
            commandBuffer.bindPipeline(pipelineType, pipeline.get());
            (*renderOperations)(commandBuffer);
            
        }
        void Execute(vk::CommandBuffer commandBuffer)
        {
            for (int i = 0; i < tasks.size(); ++i)
            {
                if (tasks[i]!= nullptr)
                {
                    (*tasks[i])();
                }
            }
            switch (pipelineType)
            {
            case vk::PipelineBindPoint::eGraphics:
                ExecutePass(commandBuffer);
                break;
            case vk::PipelineBindPoint::eCompute:
                ExecuteCompute(commandBuffer);
                break;
            default:
                assert(false && "Unsuported pipeline type");
                break;
            }
        }

        void SetVertexInput(VertexInput vertexInput)
        {
            this->vertexInput = vertexInput;
            
        }
        void SetFramebufferSize(glm::uvec2 size)
        {
            this->frameBufferSize = size;
        }
        void SetRenderOperation(std::function<void(vk::CommandBuffer& commandBuffer)>* renderOperations)
        {
            this->renderOperations =renderOperations;
        }
        void AddTask(std::function<void()>* task)
        {
            this->tasks.push_back(task);
        }
        
        void SetPipelineLayoutCI(vk::PipelineLayoutCreateInfo createInfo)
        {
            this->pipelineLayoutCI = createInfo;
            
        }
        void SetDepthConfig(DepthConfigs dephtConfig)
        {
            depthConfig = dephtConfig;
        }
        void AddColorBlendConfig(BlendConfigs blendConfig)
        {
            colorBlendConfigs.push_back(blendConfig);
        }

        void SetFragModule(ShaderModule* fragShaderModule)
        {
            this->fragShaderModule = fragShaderModule;
        }

        void SetVertModule(ShaderModule* fragShaderModule)
        {
            this->vertShaderModule = fragShaderModule;
        }

        void SetCompModule(ShaderModule* fragShaderModule)
        {
            this->compShaderModule = fragShaderModule;
        }
        void AddColorAttachmentInput(std::string name)
        {
            if (outColAttachmentsProxyRef->contains(name))
            {
                colAttachments.push_back(outColAttachmentsProxyRef->at(name));
            }else
            {
                std::cout << "Attachment input: " << "\""<< name << "\"" << " does not exist";
            }
            
        }
        void AddColorAttachmentOutput(std::string name, AttachmentInfo attachmentInfo)
        {
            if (!outColAttachmentsProxyRef->contains(name))
            {
                outColAttachmentsProxyRef->try_emplace(name, attachmentInfo);
                colAttachments.push_back(outColAttachmentsProxyRef->at(name));
            }
            else
            {
                std::cout << "Attachment: " << "\"" << name << "\"" << " already exist";
            }
            
        }

        void SetDepthAttachmentInput(std::string name)
        {
            if (outDepthAttachmentProxyRef->contains(name))
            {
                depthAttachment = outDepthAttachmentProxyRef->at(name);
            }else
            {
                std::cout << "Attachment input: " << "\""<< name << "\"" << " does not exist";
            }
            
        }
        void SetDepthAttachmentOutput(std::string name, AttachmentInfo depth)
        {
            if (!outDepthAttachmentProxyRef->contains(name))
            {
                outDepthAttachmentProxyRef->try_emplace(name, depth);
                depthAttachment = outColAttachmentsProxyRef->at(name);
            }
            else
            {
                std::cout << "Attachment: " << "\"" << name << "\"" << " already exist";
            }
        }
        void AddNodeColAttachmentImg(std::string name, ImageView* imageView)
        {
            if (!imagesAttachment.contains(name))
            {
                imagesAttachment.try_emplace(name, imageView);
            }else
            {
                imagesAttachment.at(name)= imageView;
            }
        }
        void AddNodeSampler(std::string name, ImageView* imageView)
        {
            if (!sampledImages.contains(name))
            {
                sampledImages.try_emplace(name, imageView);
            }else
            {
                sampledImages.at(name)= imageView;
            }
        }

        void AddNodeStorageImg(std::string name, ImageView* imageView)
        {
            if (!storageImages.contains(name))
            {
                storageImages.try_emplace(name, imageView);
            }
            else
            {
                storageImages.at(name) = imageView;
            }
        }
        void DependsOn(std::string dependency)
        {
            if (!dependencies.contains(dependency))
            {
                dependencies.insert(dependency);
            }else
            {
                std::cout << "Renderpass \""<<this->passName<<" Already depends on \"" <<dependency <<"\" \n";
            }
        }
        //this resources wiil be invalid at runtime
        void ClearUnusedResources()
        {
            vertShaderModule = nullptr;
            fragShaderModule = nullptr;
            compShaderModule = nullptr;
            colorBlendConfigs.clear();
            depthConfig = D_NONE;
            colAttachments.clear();
            tasks.clear();
            renderOperations = nullptr;
        }
         void ClearOperations()
        {
            delete renderOperations;
            for (auto& task : tasks)
            {
                delete task;
            }
            renderOperations = nullptr;
            tasks.clear();
        }       
        
        
        vk::UniquePipeline pipeline;
        vk::UniquePipelineLayout pipelineLayout;
        vk::UniquePipelineCache pipelineCache;
        vk::PipelineLayoutCreateInfo pipelineLayoutCI;
        vk::PipelineBindPoint pipelineType;
        DynamicRenderPass dynamicRenderPass;
        
    private:
        friend class RenderGraph;
        ShaderModule* vertShaderModule = nullptr;
        ShaderModule* fragShaderModule = nullptr;
        ShaderModule* compShaderModule = nullptr;
        std::vector<BlendConfigs> colorBlendConfigs;
        DepthConfigs depthConfig;
        VertexInput vertexInput;
        glm::uvec2 frameBufferSize;
        
        std::vector<AttachmentInfo> colAttachments;
        AttachmentInfo depthAttachment;
        
        ImageView* depthImage = nullptr;
        std::map<std::string,ImageView*> imagesAttachment;
        std::map<std::string,ImageView*> storageImages;
        std::map<std::string,ImageView*> sampledImages;
        
        std::function<void(vk::CommandBuffer& commandBuffer)>* renderOperations = nullptr;
        std::vector<std::function<void()>*> tasks;

        std::string passName;
        std::set<std::string> dependencies;
        
        Core* core;
        std::map<std::string, ImageView*>* imagesProxyRef;
        std::map<std::string, AttachmentInfo>* outColAttachmentsProxyRef;
        std::map<std::string, AttachmentInfo>* outDepthAttachmentProxyRef;
        
    };


    class RenderGraph
    {
    public:
        std::map<std::string, std::unique_ptr<RenderGraphNode>> renderNodes;
        std::vector<RenderGraphNode*> renderNodesSorted;
        std::map<std::string, ImageView*> imagesProxy;
        
        std::map<std::string, AttachmentInfo> outColAttachmentsProxy;
        
        std::map<std::string, AttachmentInfo> outDepthAttachmentProxy;
        Core* core;
        RenderGraph(Core* core)
        {
            this->core = core;
        }
        ~RenderGraph()
        {
            
        }
        RenderGraphNode* GetNode(std::string name)
        {
            if (renderNodes.contains(name))
            {
                return renderNodes.at(name).get();
            }else
            {
                PrintInvalidResource("Renderpass", name);
                return nullptr;
            }
        }

        RenderGraphNode* AddPass(std::string name)
        {
            if (!renderNodes.contains(name))
            {
                auto renderGraphNode = std::make_unique<RenderGraphNode>();
                renderGraphNode->imagesProxyRef = &imagesProxy;
                renderGraphNode->outColAttachmentsProxyRef = &outColAttachmentsProxy;
                renderGraphNode->outDepthAttachmentProxyRef = &outColAttachmentsProxy;
                renderGraphNode->core = core;
                
                renderNodes.try_emplace(name,std::move(renderGraphNode));
                return renderNodes.at(name).get();
            }else
            {
                return nullptr;
            }
        }
        ImageView* AddImageResource(std::string passName,std::string name, ImageView* imageView)
        {
            assert(imageView && "ImageView is null");
            if (!imagesProxy.contains(name))
            {
                imagesProxy.try_emplace(name, imageView);
                if (renderNodes.contains(passName))
                {
                    renderNodes.at(passName)->AddNodeColAttachmentImg(name, imageView);
                }
                else
                {
                    std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
                }
            }else
            {
                imagesProxy.at(name) = imageView;
                if (renderNodes.contains(passName))
                {
                    renderNodes.at(passName)->AddNodeColAttachmentImg(name, imageView);
                }else
                {
                    std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
                }
                // std::cout << "Image with name: \"" << name << "\" has changed \n";
            }
            return imageView;
        }
        ImageView* AddSamplerResource(std::string passName,std::string name, ImageView* imageView)
        {
            assert(imageView && "ImageView is null");
            if (!imagesProxy.contains(name))
            {
                imagesProxy.try_emplace(name, imageView);
                if (renderNodes.contains(passName))
                {
                    renderNodes.at(passName)->AddNodeSampler(name, imageView);
                }
                else
                {
                    std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
                }
            }else
            {
                imagesProxy.at(name) = imageView;
                if (renderNodes.contains(passName))
                {
                    renderNodes.at(passName)->AddNodeSampler(name, imageView);
                }else
                {
                    std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
                }
                // std::cout << "Image with name: \"" << name << "\" has changed \n";
            }
            return imageView;
        }

        ImageView* AddStorageResource(std::string passName, std::string name, ImageView* imageView)
        {
            assert(imageView && "ImageView is null");
            if (!imagesProxy.contains(name))
            {
                imagesProxy.try_emplace(name, imageView);
                if (renderNodes.contains(passName))
                {
                    renderNodes.at(passName)->AddNodeStorageImg(name, imageView);
                }
                else
                {
                    std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
                }
            }else
            {
                imagesProxy.at(name) = imageView;
                if (renderNodes.contains(passName))
                {
                    renderNodes.at(passName)->AddNodeStorageImg(name, imageView);
                }else
                {
                    std::cout << "Renderpass: " << passName << " does not exist, saving the image anyways. \n";
                }
                // std::cout << "Image with name: \"" << name << "\" has changed \n";
            }
            return imageView;
        }

        void ExecuteAll(FrameResources* currentFrame)
        {
            assert(currentFrame && "Current frame reference is null");
            RenderGraphNode* lastNode = nullptr;
            for (auto& renderNode : renderNodes)
            {
                RenderGraphNode* node = renderNode.second.get();
                if (lastNode != nullptr)
                {
                    if (node->dependencies.contains(lastNode->passName))
                    {

                        BufferUsageTypes lastNodeType = (lastNode->pipelineType == vk::PipelineBindPoint::eGraphics) ? B_GRAPHICS_WRITE :  B_COMPUTE_WRITE;
                        BufferUsageTypes currNodeType = (lastNode->pipelineType == vk::PipelineBindPoint::eGraphics) ? B_GRAPHICS_WRITE :  B_COMPUTE_WRITE;
                        BufferAccessPattern lastNodePattern = GetSrcBufferAccessPattern(lastNodeType);
                        BufferAccessPattern currNodePattern = GetSrcBufferAccessPattern(currNodeType);
                        CreateMemBarrier(lastNodePattern, currNodePattern, currentFrame->commandBuffer.get());
                    }
                }
                node->Execute(currentFrame->commandBuffer.get());
                lastNode = node;
            }
        }
    };
}


#endif //RENDERGRAPH_HPP
