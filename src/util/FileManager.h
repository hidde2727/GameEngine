#ifndef ENGINE_UTIL_RESOURCE_MANAGER_H
#define ENGINE_UTIL_RESOURCE_MANAGER_H

#include "core/PCH.h"
#include "util/Hashing.h"

namespace Engine {
namespace Util {

    typedef std::string CacheID;

    /** 
     * Is created with a prioritized list of searchPaths, 
     * When you are trying to get a file it will return the file from the first searchPath that contains the file.
     * 
     * @warning cacheID should be an identifier ending with a file extension (using the relative path of the original file as cacheID is recommended)
     */
    class FileManager {
    public:

        /**
         * Give a prioritized list of searchPaths to initilize the filemanager.
         * First in the searchPaths is also the first to be searched, if a file is found at the search path
         * then that file is returned.
         * 
         * @name Init
         */
        ///@{
        static void Init(const std::initializer_list<std::string> searchPaths, const std::string cacheFolder);
        static void Init(const std::vector<std::string>& searchPaths, const std::string cacheFolder);
        ///@}

        /**
         * Utility functions to make caching easier. cacheID is put through a hash to get a location in the cache folder.
         * cacheID is a string.
         * 
         * @warning cacheID should end with a file extension
         * @name Cache
         */
        ///@{
        static bool DoesCacheFileExist(const CacheID cacheID);
        static std::filesystem::file_time_type GetLastCacheChange(const CacheID cacheID);
        /// Will check if one of the inputFilePaths changed after the cacheID was created
        static bool CanUseCache(const CacheID cacheID, const std::initializer_list<std::string> inputFiles);
        /// Will return the ifstream of the cached file
        static std::ifstream GetCachedFile(const CacheID cacheID, const std::ios_base::openmode inputMode = std::ios_base::in);
        /// Will return the size of the cached item
        static size_t GetCachedFileSize(const CacheID cacheID);
        /// Will create the cache file and return the ofstream
        static std::ofstream AddCachedFile(const CacheID cacheID, const std::ios_base::openmode outputMode = std::ios_base::out);
        template<class T>
        static void ReadCacheFile(const CacheID cacheID, T* readBuffer, const size_t readBufferSize, const std::ios_base::openmode inputMode = std::ios_base::in);
        template<class T>
        static void ReadCacheFile(const CacheID cacheID, std::vector<T>& readBuffer);
        static void ReadCacheFile(const CacheID cacheID, std::string& readBuffer);
        ///@}

        /**
         * Utility functions to make file storage easier.
         * 
         * @name Files
         */
        ///@{
        /// Same as std::filesystem::exists
        static bool DoesFileExists(const std::string path);
        /// Same as std::filesystem::is_regular_file
        static bool IsFileRegular(const std::string path);
        static size_t GetFileSize(const std::string path);
        static std::ifstream GetFile(const std::string path, const std::ios_base::openmode inputMode = std::ios_base::in);
        template<class T>
        static void ReadFile(const std::string path, T* readBuffer, const size_t readBufferSize, const std::ios_base::openmode inputMode = std::ios_base::in);
        template<class T>
        static void ReadFile(const std::string path, std::vector<T>& readBuffer);
        static void ReadFile(const std::string path, std::string& readBuffer);
        ///@}

        /**
         * Utility functions to make reading images easier.
         * 
         * @name Images
         */
        ///@{
        static bool GetImageInfo(const std::string path, int *x, int *y, int *comp);
        static std::shared_ptr<uint8_t> ReadImageFile(const std::string path, int *x, int *y, int *channelsInFile, int desiredChannels=4);
        static std::string GetImageReadError();
        ///@}

        /// @warning INTERNAL
        static std::string GetCacheLocation(const std::string cacheID);
        /// @warning INTERNAL
        static std::string GetFileLocation(const std::string file);
    
    private:

        static std::vector<std::filesystem::path> _searchPaths;
        static std::filesystem::path _cacheFolder;
    };





    template<class T>
    void FileManager::ReadCacheFile(const std::string cacheID, T* readBuffer, const size_t readBufferSize, const std::ios_base::openmode inputMode) {
        std::ifstream inputStream = GetCachedFile(cacheID, inputMode);
        ASSERT(!inputStream.fail(), "[Util::FileManager] Failed to open input stream for cacheID '" + cacheID + "'")
        inputStream.read(reinterpret_cast<char*>(readBuffer), readBufferSize*sizeof(T));
        inputStream.close();
    }
    template<class T>
    void FileManager::ReadCacheFile(const std::string cacheID, std::vector<T>& readBuffer) {
        readBuffer.resize(std::ceil(GetCachedFileSize(cacheID) / sizeof(T)));
        ReadCacheFile(cacheID, readBuffer.data(), readBuffer.size(), std::ios::binary);
    }


    template<class T>
    void FileManager::ReadFile(const std::string path, T* readBuffer, const size_t readBufferSize, const std::ios_base::openmode inputMode) {
        std::ifstream inputStream = GetFile(path, inputMode);
        ASSERT(!inputStream.fail(), "[Util::FileManager] Failed to open input stream for file '" + path + "'")
        inputStream.read(reinterpret_cast<char*>(readBuffer), readBufferSize*sizeof(T));
        inputStream.close();
    }
    template<class T>
    void FileManager::ReadFile(const std::string path, std::vector<T>& readBuffer) {
        readBuffer.resize(GetFileSize(path));
        ReadFile(path, readBuffer.data(), readBuffer.size(), std::ios::binary);
    }


    

}
}

#endif