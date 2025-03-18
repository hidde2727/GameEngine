#include "renderer/vulkan/Pipeline.h"

namespace Engine {
namespace Renderer {
namespace Vulkan {
    
    void Pipeline::Init(PipelineCreator info, const Context context) {
        std::vector<VkShaderModule> shaderModules(info._shaderInfos.size());
        for(size_t i = 0; i < info._shaderInfos.size(); i++) {
            VkResult result = vkCreateShaderModule(context._device, &info._shaderInfos[i], nullptr, &shaderModules[i]);
            ASSERT(result != VK_SUCCESS, "Failed to create vulkan shader module");
            info._shaderStageInfos[i].module = shaderModules[i];
        }


        for(const VkShaderModule module : shaderModules) {
            vkDestroyShaderModule(context._device, module, nullptr);
        }
    }
    void Pipeline::Cleanup(const Context context) {

    }

    void PipelineCreator::SetShaders(const std::initializer_list<const char*> fileLocations) {
        _shaders.resize(fileLocations.size());
        _shaderInfos.resize(fileLocations.size());
        _shaderStageInfos.resize(fileLocations.size());

        int i = 0;
        for(const char* fileLocation : fileLocations) {
            ASSERT(!std::filesystem::exists(fileLocation), "Specified vulkan shader file does not exist");
            const std::string filetype = std::filesystem::path(fileLocation).extension().string();
            VkShaderStageFlagBits stageType = VK_SHADER_STAGE_VERTEX_BIT;
            if(filetype == "frag") stageType = VK_SHADER_STAGE_FRAGMENT_BIT;
            else if(filetype == "comp") stageType = VK_SHADER_STAGE_COMPUTE_BIT;
            else if(filetype == "geom") stageType = VK_SHADER_STAGE_GEOMETRY_BIT;
            // First check the cached versions
            std::string chacheName = "./resources/engine/chache/" + Util::Base64FileEncode(Util::SHA1(fileLocation)) + ".spiv";
            if(std::filesystem::exists(chacheName)) {
                std::filesystem::file_time_type lastFileChange = std::filesystem::last_write_time(fileLocation);
                std::filesystem::file_time_type lastCacheChange = std::filesystem::last_write_time(chacheName);
                
                if(lastFileChange < lastCacheChange) {
                    // Use the cache
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
            }
            INFO("Compiling vulkan shader - " + std::string(fileLocation));

            // Else generate the shader
            std::string shaderCode;
            std::ifstream inputStream(fileLocation);
            ASSERT(inputStream.fail(), ("Failed to open output stream for file '" + std::string(fileLocation) + "' because : \n\t" + std::string(strerror(errno))).c_str());
            shaderCode.resize(inputStream.tellg());
            inputStream.read(shaderCode.data(), shaderCode.size());

            glslang::TShader shader(EShLangVertex);
            const char* sources[1] = { shaderCode.c_str() };
            shader.setStrings(sources, 1);

            glslang::EShTargetClientVersion targetApiVersion = glslang::EShTargetVulkan_1_3;
            shader.setEnvClient(glslang::EShClientVulkan, targetApiVersion);
            glslang::EShTargetLanguageVersion spirvVersion = glslang::EShTargetSpv_1_3;
            shader.setEnvTarget(glslang::EshTargetSpv, spirvVersion);

            shader.setEntryPoint("main");
            const TBuiltInResource* resources = GetDefaultResources();
            const int defaultVersion = 450;
            const bool forwardCompatible = false;
            const EShMessages messageFlags = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
            EProfile defaultProfile = ENoProfile;

            glslang::TShader::ForbidIncluder forbidInclude;

            LOG("Preprocessing vulkan shader")
            std::string preprocessedStr;
            if ( !shader.preprocess(resources, defaultVersion, defaultProfile, false, forwardCompatible, messageFlags, &preprocessedStr, forbidInclude) )
                THROW(("Failed to preprocess vulkan shader :" + std::string(shader.getInfoLog())).c_str())
            
            const char* preprocessedSources[1] = { preprocessedStr.c_str() };
            shader.setStrings(preprocessedSources, 1);

            LOG("Parsing vulkan shader")
            if ( !shader.parse(resources, defaultVersion, defaultProfile, false, forwardCompatible, messageFlags, forbidInclude) )
                THROW(("Failed to parse vulkan shader :" + std::string(shader.getInfoLog())).c_str())

            LOG("Converting vulkan shader to spv")
            glslang::TIntermediate& intermediateRef = *(shader.getIntermediate());
            glslang::SpvOptions options{};
            options.validate = true;
            glslang::GlslangToSpv(intermediateRef, _shaders[i], &options);
            LOG("Done compiling shader")

            _shaderInfos[i].sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            _shaderInfos[i].codeSize = _shaders[i].size() * sizeof(uint32_t);
            _shaderInfos[i].pCode = _shaders[i].data();

            _shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            _shaderStageInfos[i].stage = stageType;
            _shaderStageInfos[i].module = nullptr;
            _shaderStageInfos[i].pName = "main";

            // Write the generated program to the cache
            {
                std::ofstream outputStream(chacheName.c_str(), std::ios::app); // Create the file if not exists
                ASSERT(outputStream.fail(), ("Failed to create file '" + chacheName + "' because : \n\t" + std::string(strerror(errno))).c_str());
                outputStream.close();
            }
            std::ofstream outputStream(chacheName.c_str(), std::ios::trunc | std::ios::binary);
            ASSERT(outputStream.fail(), ("Failed to open output stream for file '" + chacheName + "' because : \n\t" + std::string(strerror(errno))).c_str());
            outputStream.write(reinterpret_cast<char*>(_shaders[i].data()), _shaders[i].size()*sizeof(uint32_t));
            outputStream.close();

            i++;
        }
    }
    
}
}
}