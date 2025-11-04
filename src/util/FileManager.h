#ifndef ENGINE_UTIL_RESOURCE_MANAGER_H
#define ENGINE_UTIL_RESOURCE_MANAGER_H

#include "core/PCH.h"
#include "util/Hashing.h"

namespace Engine {
namespace Util {

    // Is created with a prioritized list of folders, 
    //      if trying to get a file it will return the file from the first folder that contains the file
    // ! cacheID should be an identifier ending with a file extension (should just use the relative path of the original file as cacheID)
    class FileManager {
    public:

        // Init
        void Init(const std::initializer_list<std::string> folders, const std::string cacheFolder);
        void Init(const std::vector<std::string>& folders, const std::string cacheFolder);

        // Cache

        bool DoesCacheFileExist(const std::string cacheID) const;
        std::filesystem::file_time_type GetLastCacheChange(const std::string cacheID) const;
        // Will check if one of the inputFilePaths changed after the cacheID was created
        bool CanUseCache(const std::string cacheID, const std::initializer_list<std::string> inputFiles) const;
        // Will return the ifstream of the cached file
        std::ifstream GetCachedFile(const std::string cacheID, const std::ios_base::openmode inputMode = std::ios_base::in) const;
        // Will return the size of the cached item
        size_t GetCachedFileSize(const std::string cacheID) const;
        // Will create the cache file and return the ofstream
        std::ofstream AddCachedFile(const std::string cacheID, const std::ios_base::openmode outputMode = std::ios_base::out) const;
        template<class T>
        void ReadCacheFile(const std::string cacheID, T* readBuffer, const size_t readBufferSize, const std::ios_base::openmode inputMode = std::ios_base::in) const {
            std::ifstream inputStream = GetCachedFile(cacheID, inputMode);
            ASSERT(!inputStream.fail(), "[Util::FileManager] Failed to open input stream for cacheID '" + cacheID + "'")
            inputStream.read(reinterpret_cast<char*>(readBuffer), readBufferSize*sizeof(T));
            inputStream.close();
        }
        template<class T>
        void ReadCacheFile(const std::string cacheID, std::vector<T>& readBuffer) const {
            readBuffer.resize(std::ceil(GetCachedFileSize(cacheID) / sizeof(T)));
            ReadCacheFile(cacheID, readBuffer.data(), readBuffer.size(), std::ios::binary);
        }
        void ReadCacheFile(const std::string cacheID, std::string& readBuffer) const {
            readBuffer.resize(GetCachedFileSize(cacheID));
            ReadCacheFile(cacheID, readBuffer.data(), readBuffer.size());
        }

        // Files

        // Will run std::filesystem::exists
        bool DoesFileExists(const std::string path) const;
        // Will run std::filesystem::is_regular_file
        bool IsFileRegular(const std::string path) const;
        size_t GetFileSize(const std::string path) const;
        std::ifstream GetFile(const std::string path, const std::ios_base::openmode inputMode = std::ios_base::in) const;
        template<class T>
        void ReadFile(const std::string path, T* readBuffer, const size_t readBufferSize, const std::ios_base::openmode inputMode = std::ios_base::in) const {
            std::ifstream inputStream = GetFile(path, inputMode);
            ASSERT(!inputStream.fail(), "[Util::FileManager] Failed to open input stream for file '" + path + "'")
            inputStream.read(reinterpret_cast<char*>(readBuffer), readBufferSize*sizeof(T));
            inputStream.close();
        }
        template<class T>
        void ReadFile(const std::string path, std::vector<T>& readBuffer) const {
            readBuffer.resize(GetFileSize(path));
            ReadFile(path, readBuffer.data(), readBuffer.size(), std::ios::binary);
        }
        void ReadFile(const std::string path, std::string& readBuffer) const {
            readBuffer.resize(GetFileSize(path));
            ReadFile(path, readBuffer.data(), readBuffer.size());
        }

        bool GetImageInfo(const std::string path, int *x, int *y, int *comp) const {
            return stbi_info(GetFileLocation(path).c_str(), x, y, comp);
        }
        uint8_t* ReadImageFile(const std::string path, int *x, int *y, int *channelsInFile, int desiredChannels=4) const {
            uint8_t* data = stbi_load(GetFileLocation(path).c_str(), x, y, channelsInFile, desiredChannels);
            return data;
        }
        std::string GetImageReadError() const {
            return std::string(stbi_failure_reason());
        }

        // INTERNAL
        std::string GetCacheLocation(const std::string cacheID) const;
        // INTERNAL
        std::string GetFileLocation(const std::string file) const;
    
    private:

        std::vector<std::filesystem::path> _folders;
        std::filesystem::path _cacheFolder;
    };

}
}

#endif