#include "util/FileManager.h"

namespace Engine {
namespace Util {

    std::vector<std::filesystem::path> FileManager::_searchPaths;
    std::filesystem::path FileManager::_cacheFolder;

    void FileManager::Init(const std::initializer_list<std::string> searchPaths, const std::string cacheFolder) {
        INFO("[Util::FileManager] Running in directory '" + std::filesystem::current_path().filename().generic_string() + "'")
        for(std::string path : searchPaths) {
            _searchPaths.push_back(std::filesystem::path(path));
        }
        _cacheFolder = cacheFolder;
        if(std::filesystem::exists(_cacheFolder)) return;
        try {
            std::filesystem::create_directory(_cacheFolder);
        } catch(std::filesystem::filesystem_error exc) {
            WARNING("[Util::FileManager] Failed to create the cache directory (" + std::string(exc.what()) + ")")
        }
        LOG("[Util::FileManager] Created cache directory at '" + std::filesystem::absolute(std::filesystem::path(_cacheFolder)).string() + "'")
    }
    void FileManager::Init(const std::vector<std::string>& searchPaths, const std::string cacheFolder) {
        INFO("[Util::FileManager] Running in directory '" + std::filesystem::current_path().filename().generic_string() + "'")
        for(std::string path : searchPaths) {
            _searchPaths.push_back(std::filesystem::path(path));
        }
        _cacheFolder = cacheFolder;
        if(std::filesystem::exists(_cacheFolder)) return;
        try {
            std::filesystem::create_directory(_cacheFolder);
        } catch(std::filesystem::filesystem_error exc) {
            WARNING("[Util::FileManager] Failed to create the cache directory (" + std::string(exc.what()) + ")")
        }
        LOG("[Util::FileManager] Created cache directory at '" + std::filesystem::absolute(std::filesystem::path(_cacheFolder)).string() + "'")
    }
    
    bool FileManager::DoesCacheFileExist(const std::string cacheID) {
        std::string cacheLocation = GetCacheLocation(cacheID);
        return std::filesystem::exists(cacheLocation);
    }

    std::filesystem::file_time_type FileManager::GetLastCacheChange(const std::string cacheID) {
        std::string cacheLocation = GetCacheLocation(cacheID);
        ASSERT(std::filesystem::exists(cacheLocation), "[Util::FileManager] Cannot get the last cache change of a item that is not cached")
        return std::filesystem::last_write_time(cacheLocation);
    }

    bool FileManager::CanUseCache(const std::string cacheID, const std::initializer_list<std::string> cacheFileDependency) {
        for(std::string filePath : cacheFileDependency) {
            std::string fileLocation = GetFileLocation(filePath);
            ASSERT(std::filesystem::exists(fileLocation), "[Util::Filemanager] Specified cache dependency does not exists (FileManager)")
            std::string cacheLocation = GetCacheLocation(cacheID);
            if(!std::filesystem::exists(cacheLocation)) return false;
            std::filesystem::file_time_type lastFileChange = std::filesystem::last_write_time(fileLocation);
            std::filesystem::file_time_type lastCacheChange = std::filesystem::last_write_time(cacheLocation);
            if(lastFileChange > lastCacheChange) return false;
        }
        return true;
    }

    std::ifstream FileManager::GetCachedFile(const std::string cacheID, const std::ios_base::openmode inputMode) {
        std::ifstream inputStream(GetCacheLocation(cacheID), inputMode);
        ASSERT(!inputStream.fail(), ("[Util::Filemanager] Failed to open input stream for cahceID '" + cacheID + "' because : \n\t" + std::string(strerror(errno))).c_str());
        return inputStream;
    }

    size_t FileManager::GetCachedFileSize(const std::string cacheID) {
        return std::filesystem::file_size(GetCacheLocation(cacheID));
    }

    std::ofstream FileManager::AddCachedFile(const std::string cacheID, std::ios_base::openmode outputMode) {
        std::string cacheLocation = GetCacheLocation(cacheID);
        {
            std::ofstream outputStream(cacheLocation.c_str(), std::ios::app); // Create the file if not exists
            ASSERT(!outputStream.fail(), ("[Util::Filemanager] Failed to create file '" + cacheLocation + "' because : \n\t" + std::string(strerror(errno))).c_str());
            outputStream.close();
        }
        std::ofstream outputStream(cacheLocation.c_str(), outputMode);
        ASSERT(!outputStream.fail(), ("[Util::Filemanager] Failed to open output stream for file '" + cacheID + "' because : \n\t" + std::string(strerror(errno))).c_str());
        return outputStream;
    }
    void FileManager::ReadCacheFile(const std::string cacheID, std::string& readBuffer) {
        readBuffer.resize(GetCachedFileSize(cacheID));
        ReadCacheFile(cacheID, readBuffer.data(), readBuffer.size());
    }




    

    bool FileManager::DoesFileExists(const std::string path) {
        try {
            GetFileLocation(path);
            return true;
        } catch(std::exception exc) {
            return false;
        }
    }
    bool FileManager::IsFileRegular(const std::string path) {
        std::string fileLocation = GetFileLocation(path);
        return std::filesystem::is_regular_file(fileLocation);
    }
    size_t FileManager::GetFileSize(const std::string path) {
        return std::filesystem::file_size(GetFileLocation(path));
    }
    std::ifstream FileManager::GetFile(const std::string path, const std::ios_base::openmode inputMode) {
        std::ifstream inputStream(GetFileLocation(path), inputMode);
        ASSERT(!inputStream.fail(), ("[Util::Filemanager] Failed to open input stream for file '" + GetFileLocation(path) + "' because : \n\t" + std::string(strerror(errno))).c_str());
        return inputStream;
    }
    void FileManager::ReadFile(const std::string path, std::string& readBuffer) {
        readBuffer.resize(GetFileSize(path));
        ReadFile(path, readBuffer.data(), readBuffer.size());
    }





    bool FileManager::GetImageInfo(const std::string path, int *x, int *y, int *comp) {
        return stbi_info(GetFileLocation(path).c_str(), x, y, comp);
    }
    std::shared_ptr<uint8_t> FileManager::ReadImageFile(const std::string path, int *x, int *y, int *channelsInFile, int desiredChannels) {
        uint8_t* data = stbi_load(GetFileLocation(path).c_str(), x, y, channelsInFile, desiredChannels);
        return std::shared_ptr<uint8_t>(data, stbi_image_free);
    }
    std::string FileManager::GetImageReadError() {
        return std::string(stbi_failure_reason());
    }
    
    std::string FileManager::GetCacheLocation(const std::string cacheID) {
        std::string cacheFile = Util::Base64FileEncode(Util::SHA1(cacheID)) + std::filesystem::path(cacheID).extension().string();
        return (_cacheFolder / std::filesystem::path(cacheFile)).string();
    }
    std::string FileManager::GetFileLocation(const std::string file) {
        // Remove leading /
        size_t firstNonSlash = file.find_first_not_of("/\\");
        std::string fileTrimmed = file.substr(firstNonSlash, file.size()-firstNonSlash);
        for(std::filesystem::path path : _searchPaths) {
            std::filesystem::path location = path / std::filesystem::path(fileTrimmed);
            if(std::filesystem::exists(location)) {
                return location.string();
            }
        }
        throw std::runtime_error("[Util::Filemanager] Cannot retrieve the file location of file '" + file + "'");
    }

}
}