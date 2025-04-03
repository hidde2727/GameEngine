#include "renderer/vulkan/Pipeline.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {
    
    void Pipeline::Init(PipelineCreator& info, const Context& context, const RenderPass& renderPass, const uint32_t subpass) {
        std::vector<VkShaderModule> shaderModules(info._shaderInfos.size());
        for(size_t i = 0; i < info._shaderInfos.size(); i++) {
            VkResult result = vkCreateShaderModule(context._device, &info._shaderInfos[i], nullptr, &shaderModules[i]);
            ASSERT(result != VK_SUCCESS, "Failed to create vulkan shader module");
            info._shaderStageInfos[i].module = shaderModules[i];
        }
        
        VkResult result = vkCreateDescriptorSetLayout(context._device, &info._descriptorLayoutInfo, nullptr, &_descriptorSetLayout);
        ASSERT(result != VK_SUCCESS, "Failed to create vulkan descriptor set layout")

        info._pipelineLayoutInfo.setLayoutCount = 1;
        info._pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
        result = vkCreatePipelineLayout(context._device, &info._pipelineLayoutInfo, nullptr, &_pipelineLayout);
        ASSERT(result != VK_SUCCESS, "Failed to create vulkan pipeline layout");

        info._pipelineInfo.stageCount = (uint32_t)info._shaderStageInfos.size();
        info._pipelineInfo.pStages = info._shaderStageInfos.data();
        info._pipelineInfo.renderPass = renderPass._renderPass;
        info._pipelineInfo.subpass = subpass;
        info._pipelineInfo.layout = _pipelineLayout;

        result = vkCreateGraphicsPipelines(context._device, VK_NULL_HANDLE, 1, &info._pipelineInfo, nullptr, &_pipeline);
        ASSERT(result != VK_SUCCESS, "Failed to create vulkan pipeline");

        for(const VkShaderModule module : shaderModules) {
            vkDestroyShaderModule(context._device, module, nullptr);
        }

        for(uint32_t i = 0; i < info._amountTextures; i++) { _availableTextureSlots.push(i); }

        result = vkCreateDescriptorPool(context._device, &info._descriptorPoolInfo, nullptr, &_descriptorPool);
        ASSERT(result != VK_SUCCESS, "Failed to create vulkan descriptor pool")

        std::vector<VkDescriptorSetLayout> layouts(info._descriptorPoolInfo.maxSets, _descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = info._descriptorPoolInfo.maxSets;
        allocInfo.pSetLayouts = layouts.data();
        _descriptorSets.resize(info._descriptorPoolInfo.maxSets);
        result = vkAllocateDescriptorSets(context._device, &allocInfo, _descriptorSets.data());
    }

    void Pipeline::BindSamplerDescriptor(const Context& context, const TextureSampler sampler, const uint32_t arrayElement) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = sampler._sampler;

        std::vector<VkWriteDescriptorSet> descriptorWrites(_descriptorSets.size());
        for(size_t i = 0; i < _descriptorSets.size(); i++) {
            descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[i].dstSet = _descriptorSets[i];
            descriptorWrites[i].dstBinding = ENGINE_RENDERER_VULKAN_IMAGESAMPLER_BINDING;
            descriptorWrites[i].dstArrayElement = arrayElement;
            descriptorWrites[i].descriptorCount = 1;
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            descriptorWrites[i].pImageInfo = &imageInfo;
        }
        vkUpdateDescriptorSets(context._device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
    uint32_t Pipeline::BindTextureDescriptor(const Context& context, Texture& texture) {
        ASSERT(_availableTextureSlots.size()==0, "Cannot bind another texture to vulkan pipeline, no available descriptor slots")
        uint32_t nextAvailableDescriptor = _availableTextureSlots.front();
        _availableTextureSlots.pop();

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = texture._imageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        std::vector<VkWriteDescriptorSet> descriptorWrites(_descriptorSets.size());
        for(size_t i = 0; i < _descriptorSets.size(); i++) {
            descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[i].dstSet = _descriptorSets[i];
            descriptorWrites[i].dstBinding = ENGINE_RENDERER_VULKAN_IMAGE_BINDING;
            descriptorWrites[i].dstArrayElement = nextAvailableDescriptor;
            descriptorWrites[i].descriptorCount = 1;
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            descriptorWrites[i].pImageInfo = &imageInfo;
        }
        vkUpdateDescriptorSets(context._device, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

        texture._boundDescriptorSlot = nextAvailableDescriptor;
        return nextAvailableDescriptor;
    }
    void Pipeline::UnbindTextureDescriptor(const Context& context, const uint32_t slot, Texture& texture) {
        // TODO: Check if the given slot isn't already in the availableTextureSlots
        _availableTextureSlots.push(slot);
        texture._boundDescriptorSlot = UINT32_MAX;
    }

    void Pipeline::Cleanup(const Context& context) {
        vkDestroyPipeline(context._device, _pipeline, nullptr);
        vkDestroyPipelineLayout(context._device, _pipelineLayout, nullptr);
        vkDestroyDescriptorPool(context._device, _descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(context._device, _descriptorSetLayout, nullptr);
    }




    PipelineCreator::PipelineCreator() {
        _dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        _dynamicStateInfo.dynamicStateCount = 0;
        _dynamicStateInfo.pDynamicStates = nullptr;

        _vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        _vertexInputInfo.vertexBindingDescriptionCount = 0;
        _vertexInputInfo.pVertexBindingDescriptions = nullptr;
        _vertexInputInfo.vertexAttributeDescriptionCount = 0;
        _vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        _inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        _inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        _inputAssembly.primitiveRestartEnable = VK_FALSE;

        _viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        _viewportState.viewportCount = 1;
        _viewportState.scissorCount = 1;

        _rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        _rasterizer.depthClampEnable = VK_FALSE;
        _rasterizer.rasterizerDiscardEnable = VK_FALSE;
        _rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        _rasterizer.lineWidth = 1.0f;
        _rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        _rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        _rasterizer.depthBiasEnable = VK_FALSE;
        _rasterizer.depthBiasConstantFactor = 0.0f;
        _rasterizer.depthBiasClamp = 0.0f;
        _rasterizer.depthBiasSlopeFactor = 0.0f;

        _multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        _multisampling.sampleShadingEnable = VK_FALSE;
        _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        _multisampling.minSampleShading = 1.0f;
        _multisampling.pSampleMask = nullptr;
        _multisampling.alphaToCoverageEnable = VK_FALSE;
        _multisampling.alphaToOneEnable = VK_FALSE;

        _colorBlendAttachments.resize(1);
        _colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        _colorBlendAttachments[0].blendEnable = VK_FALSE;
        _colorBlendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        _colorBlendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        _colorBlendAttachments[0].colorBlendOp = VK_BLEND_OP_ADD;
        _colorBlendAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        _colorBlendAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        _colorBlendAttachments[0].alphaBlendOp = VK_BLEND_OP_ADD;

        _colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        _colorBlending.logicOpEnable = VK_FALSE;
        _colorBlending.logicOp = VK_LOGIC_OP_COPY;
        _colorBlending.attachmentCount = (uint32_t)_colorBlendAttachments.size();
        _colorBlending.pAttachments = _colorBlendAttachments.data();
        _colorBlending.blendConstants[0] = 0.0f;
        _colorBlending.blendConstants[1] = 0.0f;
        _colorBlending.blendConstants[2] = 0.0f;
        _colorBlending.blendConstants[3] = 0.0f;

        _pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        _pipelineLayoutInfo.setLayoutCount = 0;
        _pipelineLayoutInfo.pSetLayouts = nullptr;
        _pipelineLayoutInfo.pushConstantRangeCount = 0;
        _pipelineLayoutInfo.pPushConstantRanges = nullptr;

        _pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        _pipelineInfo.stageCount = 0;
        _pipelineInfo.pStages = nullptr;
        _pipelineInfo.pVertexInputState = &_vertexInputInfo;
        _pipelineInfo.pInputAssemblyState = &_inputAssembly;
        _pipelineInfo.pViewportState = &_viewportState;
        _pipelineInfo.pRasterizationState = &_rasterizer;
        _pipelineInfo.pMultisampleState = &_multisampling;
        _pipelineInfo.pDepthStencilState = nullptr;
        _pipelineInfo.pColorBlendState = &_colorBlending;
        _pipelineInfo.pDynamicState = &_dynamicStateInfo;
        _pipelineInfo.layout = VK_NULL_HANDLE;
        _pipelineInfo.renderPass = VK_NULL_HANDLE;
        _pipelineInfo.subpass = 0;
        _pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        _pipelineInfo.basePipelineIndex = -1;
    }

    void PipelineCreator::SetShaders(const std::initializer_list<const char*> fileLocations) {
        _shaders.resize(fileLocations.size());
        _shaderInfos.resize(fileLocations.size());
        _shaderStageInfos.resize(fileLocations.size());

        // Check if we can use the cache
        bool cannotUseCache = false;
        for(const char* fileLocation : fileLocations) {
            ASSERT(!std::filesystem::exists(fileLocation), "Specified vulkan shader file does not exist");

            std::string chacheName = "resources/engine/chache/" + Util::Base64FileEncode(Util::SHA1(fileLocation)) + ".spiv";
            if(!std::filesystem::exists(chacheName)) { cannotUseCache = true; break; }
            std::filesystem::file_time_type lastFileChange = std::filesystem::last_write_time(fileLocation);
            std::filesystem::file_time_type lastCacheChange = std::filesystem::last_write_time(chacheName);
            if(lastFileChange > lastCacheChange) { cannotUseCache = true; break; }
        }

        // Use the cache
        if(!cannotUseCache) {
            int i = 0;
            for(const char* fileLocation : fileLocations) {
                std::string chacheName = "resources/engine/chache/" + Util::Base64FileEncode(Util::SHA1(fileLocation)) + ".spiv";

                const std::string filetype = std::filesystem::path(fileLocation).extension().string();
                VkShaderStageFlagBits stageType = VK_SHADER_STAGE_VERTEX_BIT;
                if(filetype == ".frag") stageType = VK_SHADER_STAGE_FRAGMENT_BIT;
                else if(filetype == ".comp") stageType = VK_SHADER_STAGE_COMPUTE_BIT;
                else if(filetype == ".geom") stageType = VK_SHADER_STAGE_GEOMETRY_BIT;

                std::ifstream inputStream(chacheName, std::ios::binary);
                _shaders[i].resize((size_t)std::ceil(std::filesystem::file_size(chacheName) / sizeof(uint32_t)));
                inputStream.read(reinterpret_cast<char*>(_shaders[i].data()), _shaders[i].size() * sizeof(uint32_t));
                inputStream.close();

                _shaderInfos[i].sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                _shaderInfos[i].codeSize = _shaders[i].size()*sizeof(uint32_t);
                _shaderInfos[i].pCode = reinterpret_cast<const uint32_t*>(_shaders[i].data());

                _shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                _shaderStageInfos[i].stage = stageType;
                _shaderStageInfos[i].module = nullptr;
                _shaderStageInfos[i].pName = "main";

                i++;
                continue;
            }
            return;
        }

        // Else compile the files
        std::unique_ptr<glslang::TProgram> program = std::make_unique<glslang::TProgram>();
        std::vector<std::unique_ptr<glslang::TShader>> shaders(fileLocations.size());

        int i = 0;
        for(const char* fileLocation : fileLocations) {
            const std::string filetype = std::filesystem::path(fileLocation).extension().string();
            EShLanguage languageType = EShLangVertex;
            if(filetype == ".frag") languageType = EShLangFragment;
            else if(filetype == ".comp") languageType = EShLangCompute;
            else if(filetype == ".geom") languageType = EShLangGeometry;
            
            INFO("Compiling vulkan shader - " + std::string(fileLocation));

            std::string shaderCode;
            std::ifstream inputStream(fileLocation);
            ASSERT(inputStream.fail(), ("Failed to open output stream for file '" + std::string(fileLocation) + "' because : \n\t" + std::string(strerror(errno))).c_str());
            shaderCode.resize(std::filesystem::file_size(fileLocation));
            inputStream.read(shaderCode.data(), shaderCode.size());

            shaders[i] = std::make_unique<glslang::TShader>(languageType);
            const char* sources[1] = { shaderCode.c_str() };
            shaders[i]->setStrings(sources, 1);

            shaders[i]->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
            shaders[i]->setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_3);
            shaders[i]->setEnvInput(glslang::EShSourceGlsl,
                languageType,
                glslang::EShClientVulkan,
                450);
            
            shaders[i]->setEntryPoint("main");
            const TBuiltInResource* resources = GetDefaultResources();
            const int defaultVersion = 450;
            const bool forwardCompatible = false;
            const EShMessages messageFlags = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
            EProfile defaultProfile = ENoProfile;

            glslang::TShader::ForbidIncluder forbidInclude;

            LOG("Preprocessing vulkan shader")
            std::string preprocessedStr;
            if ( !shaders[i]->preprocess(resources, defaultVersion, defaultProfile, false, forwardCompatible, messageFlags, &preprocessedStr, forbidInclude) )
                THROW(("Failed to preprocess vulkan shader (" + std::string(fileLocation) + ") :\n" + std::string(shaders[i]->getInfoLog())).c_str())
            
            const char* preprocessedSources[1] = { preprocessedStr.c_str() };
            shaders[i]->setStrings(preprocessedSources, 1);

            LOG("Parsing vulkan shader")
            if ( !shaders[i]->parse(resources, defaultVersion, defaultProfile, false, forwardCompatible, messageFlags, forbidInclude) )
                THROW(("Failed to parse vulkan shader (" + std::string(fileLocation) + ") :\n" + std::string(shaders[i]->getInfoLog())).c_str())

            program->addShader(shaders[i].get());
            i++;
        }

        INFO("Linking vulkan shaders")
        if( !program->link(EShMsgDefault) )
            THROW(("Failed to link vulkan shaders :" + std::string(shaders[i]->getInfoLog())).c_str())

        for(i = 0; i < shaders.size(); i++) {
            VkShaderStageFlagBits stageType = VK_SHADER_STAGE_VERTEX_BIT;
            if(shaders[i]->getStage() == EShLangFragment) stageType = VK_SHADER_STAGE_FRAGMENT_BIT;
            else if(shaders[i]->getStage() == EShLangCompute) stageType = VK_SHADER_STAGE_COMPUTE_BIT;
            else if(shaders[i]->getStage() == EShLangGeometry) stageType = VK_SHADER_STAGE_GEOMETRY_BIT;

            LOG("Converting vulkan shader to spv")
            glslang::TIntermediate& intermediateRef = *(program->getIntermediate(shaders[i]->getStage()));
            glslang::SpvOptions options{};
            options.validate = true;
            glslang::GlslangToSpv(intermediateRef, _shaders[i], &options);

            _shaderInfos[i].sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            _shaderInfos[i].codeSize = _shaders[i].size() * sizeof(uint32_t);
            _shaderInfos[i].pCode = _shaders[i].data();

            _shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            _shaderStageInfos[i].stage = stageType;
            _shaderStageInfos[i].module = nullptr;
            _shaderStageInfos[i].pName = "main";

            // Write the generated program to the cache
            std::string chacheName = "resources/engine/chache/" + Util::Base64FileEncode(Util::SHA1(*(fileLocations.begin() + i))) + ".spiv";
            {
                std::ofstream outputStream(chacheName.c_str(), std::ios::app); // Create the file if not exists
                ASSERT(outputStream.fail(), ("Failed to create file '" + chacheName + "' because : \n\t" + std::string(strerror(errno))).c_str());
                outputStream.close();
            }
            std::ofstream outputStream(chacheName.c_str(), std::ios::trunc | std::ios::binary);
            ASSERT(outputStream.fail(), ("Failed to open output stream for file '" + chacheName + "' because : \n\t" + std::string(strerror(errno))).c_str());
            outputStream.write(reinterpret_cast<char*>(_shaders[i].data()), _shaders[i].size()*sizeof(uint32_t));
            outputStream.close();
        }

        program.release();
    }

    void PipelineCreator::SetDynamicState(const std::initializer_list<VkDynamicState> dynamicState) {
        _dynamicState = dynamicState;
        _dynamicStateInfo.dynamicStateCount = (uint32_t)_dynamicState.size();
        _dynamicStateInfo.pDynamicStates = _dynamicState.data();
    }

    void PipelineCreator::SetVertexInput(const std::initializer_list<Vertex::Attribute> perVertex, const std::initializer_list<Vertex::Attribute> perInstance) {
        _vertexAttributes.resize(perVertex.size()+perInstance.size());
        uint32_t i = 0;

        if(perVertex.size() > 0) {
            uint32_t offset = 0;
            for(Vertex::Attribute attribute : perVertex) {
                _vertexAttributes[i].binding = 0;
                _vertexAttributes[i].location = i;
                _vertexAttributes[i].format = attribute.first;
                _vertexAttributes[i].offset = offset;
                offset += attribute.second;
                i++;
                if(attribute.second > 32) i++;
            }
    
            _vertexBinding.push_back(VkVertexInputBindingDescription{});
            _vertexBinding[0].binding = 0;
            _vertexBinding[0].stride = offset;
            _vertexBinding[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        if(perInstance.size() > 0) {
            uint32_t offset = 0;
            for(Vertex::Attribute attribute : perInstance) {
                _vertexAttributes[i].binding = 1;
                _vertexAttributes[i].location = i;
                _vertexAttributes[i].format = attribute.first;
                _vertexAttributes[i].offset = offset;
                offset += attribute.second;
                i++;
                if(attribute.second > 32) i++;
            }
    
            _vertexBinding.push_back(VkVertexInputBindingDescription{
                1, offset, VK_VERTEX_INPUT_RATE_INSTANCE
            });
        }

        _vertexInputInfo.vertexBindingDescriptionCount = _vertexBinding.size();
        _vertexInputInfo.pVertexBindingDescriptions = _vertexBinding.data();
        _vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)_vertexAttributes.size();
        _vertexInputInfo.pVertexAttributeDescriptions = _vertexAttributes.data();
    }
    

    void PipelineCreator::SetInputAssembly(const VkPrimitiveTopology topology, const VkBool32 primitiveRestart) {
        _inputAssembly.topology = topology;
        _inputAssembly.primitiveRestartEnable = primitiveRestart;
    }

    void PipelineCreator::SetViewport(const VkViewport viewport, const VkRect2D scissorRect) {
        _viewport = viewport;
        _scissorRect = scissorRect;

        _viewportState.viewportCount = 1;
        _viewportState.pViewports = &_viewport;
        _viewportState.scissorCount = 1;
        _viewportState.pScissors = &_scissorRect;
    }

    void PipelineCreator::SetPushConstantInput(const std::initializer_list<Vertex::Attribute> attributes, const VkShaderStageFlags shader) {
        uint32_t totalSize = 0;
        for(Vertex::Attribute attribute : attributes) { totalSize += attribute.second; }
        VkPushConstantRange range = {};
        range.stageFlags = shader;
        range.offset = 0;
        range.size = totalSize;
        _pushConstantRanges.push_back(range);

        _pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)_pushConstantRanges.size();
        _pipelineLayoutInfo.pPushConstantRanges = _pushConstantRanges.data();
    }

    void PipelineCreator::EnableAlphaBlending() {
        _colorBlendAttachments[0].blendEnable = VK_TRUE;
        _colorBlendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        _colorBlendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        _colorBlendAttachments[0].colorBlendOp = VK_BLEND_OP_ADD;
        _colorBlendAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        _colorBlendAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        _colorBlendAttachments[0].alphaBlendOp = VK_BLEND_OP_ADD;
    }
    
    void PipelineCreator::SetDescriptorInfo(const uint32_t duplicateSets, const uint32_t textures, const uint32_t imageSamplers, const uint32_t uniformBuffers) {
        _amountTextures = textures;
        if(textures > 0) {
            VkDescriptorPoolSize poolSize{};
            poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            poolSize.descriptorCount = textures;
            _descriptorPoolSizes.push_back(poolSize);
            VkDescriptorSetLayoutBinding layout{};

            layout.binding = ENGINE_RENDERER_VULKAN_IMAGE_BINDING;
            layout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            layout.descriptorCount = textures;
            layout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layout.pImmutableSamplers = nullptr;
            _descriptorLayoutBindings.push_back(layout);

            _descriptorLayoutBindingFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
        }
        if(imageSamplers > 0) {
            VkDescriptorPoolSize poolSize{};
            poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
            poolSize.descriptorCount = imageSamplers;
            _descriptorPoolSizes.push_back(poolSize);
            VkDescriptorSetLayoutBinding layout{};

            layout.binding = ENGINE_RENDERER_VULKAN_IMAGESAMPLER_BINDING;
            layout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            layout.descriptorCount = imageSamplers;
            layout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layout.pImmutableSamplers = nullptr;
            _descriptorLayoutBindings.push_back(layout);

            _descriptorLayoutBindingFlags.push_back(0);
        }
        if(uniformBuffers > 0) {
            VkDescriptorPoolSize poolSize{};
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSize.descriptorCount = uniformBuffers;
            _descriptorPoolSizes.push_back(poolSize);

            VkDescriptorSetLayoutBinding layout{};
            layout.binding = ENGINE_RENDERER_VULKAN_UNIFORMBUFFER_BINDING;
            layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layout.descriptorCount = uniformBuffers;
            layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            layout.pImmutableSamplers = nullptr;
            _descriptorLayoutBindings.push_back(layout);

            _descriptorLayoutBindingFlags.push_back(0);
        }
        _descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        _descriptorPoolInfo.poolSizeCount = (uint32_t)_descriptorPoolSizes.size();
        _descriptorPoolInfo.pPoolSizes = _descriptorPoolSizes.data();
        _descriptorPoolInfo.maxSets = (uint32_t)duplicateSets;
        _descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        
        _descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        _descriptorLayoutInfo.bindingCount = (uint32_t)_descriptorLayoutBindings.size();
        _descriptorLayoutInfo.pBindings = _descriptorLayoutBindings.data();

        if(textures > 0) {
            _descriptorLayoutBindingExtraInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
            _descriptorLayoutBindingExtraInfo.pNext = nullptr;
            _descriptorLayoutBindingExtraInfo.bindingCount = (uint32_t)_descriptorLayoutBindingFlags.size();
            _descriptorLayoutBindingExtraInfo.pBindingFlags = _descriptorLayoutBindingFlags.data();
            _descriptorLayoutInfo.pNext = &_descriptorLayoutBindingExtraInfo;
            _descriptorLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }

    }
    
}
}
}