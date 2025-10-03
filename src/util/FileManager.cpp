#include "util/FileManager.h"

namespace Engine {
namespace Util {


    void FileManager::Init(const std::initializer_list<std::string> folders, const std::string cacheFolder) {
        for(std::string folder : folders) {
            _folders.push_back(std::filesystem::path(folder));
        }
        _cacheFolder = cacheFolder;
        LOG("[Util::FileManager] Create cache directory at '" + std::filesystem::path(_cacheFolder).string() + "'")
        std::filesystem::create_directory(_cacheFolder);
    }
    void FileManager::Init(const std::vector<std::string>& folders, const std::string cacheFolder) {
        for(std::string folder : folders) {
            _folders.push_back(std::filesystem::path(folder));
        }
        _cacheFolder = cacheFolder;
        LOG("[Util::FileManager] Create cache directory at '" + std::filesystem::path(_cacheFolder).string() + "'")
        std::filesystem::create_directory(_cacheFolder);
    }
    
    bool FileManager::DoesCacheFileExist(const std::string cacheID) const {
        std::string cacheLocation = GetCacheLocation(cacheID);
        return std::filesystem::exists(cacheLocation);
    }

    std::filesystem::file_time_type FileManager::GetLastCacheChange(const std::string cacheID) const {
        std::string cacheLocation = GetCacheLocation(cacheID);
        ASSERT(std::filesystem::exists(cacheLocation), "[Util::FileManager] Cannot get the last cache change of a item that is not cached")
        return std::filesystem::last_write_time(cacheLocation);
    }

    bool FileManager::CanUseCache(const std::string cacheID, const std::initializer_list<std::string> cacheFileDependency) const {
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

    std::ifstream FileManager::GetCachedFile(const std::string cacheID, const std::ios_base::openmode inputMode) const {
        std::ifstream inputStream(GetCacheLocation(cacheID), inputMode);
        ASSERT(!inputStream.fail(), ("[Util::Filemanager] Failed to open input stream for cahceID '" + cacheID + "' because : \n\t" + std::string(strerror(errno))).c_str());
        return inputStream;
    }

    size_t FileManager::GetCachedFileSize(const std::string cacheID) const {
        return std::filesystem::file_size(GetCacheLocation(cacheID));
    }

    std::ofstream FileManager::AddCachedFile(const std::string cacheID, std::ios_base::openmode outputMode) const {
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


    bool FileManager::DoesFileExists(const std::string path) const {
        try {
            GetFileLocation(path);
            return true;
        } catch(std::exception exc) {
            return false;
        }
    }
    bool FileManager::IsFileRegular(const std::string path) const {
        std::string fileLocation = GetFileLocation(path);
        return std::filesystem::is_regular_file(fileLocation);
    }
    size_t FileManager::GetFileSize(const std::string path) const {
        return std::filesystem::file_size(GetFileLocation(path));
    }

    std::ifstream FileManager::GetFile(const std::string path, const std::ios_base::openmode inputMode) const {
        std::ifstream inputStream(GetFileLocation(path), inputMode);
        ASSERT(!inputStream.fail(), ("[Util::Filemanager] Failed to open input stream for file '" + GetFileLocation(path) + "' because : \n\t" + std::string(strerror(errno))).c_str());
        return inputStream;
    }
    
    std::string FileManager::GetCacheLocation(const std::string cacheID) const {
        std::string cacheFile = Util::Base64FileEncode(Util::SHA1(cacheID)) + std::filesystem::path(cacheID).extension().string();
        return (_cacheFolder / std::filesystem::path(cacheFile)).string();
    }
    std::string FileManager::GetFileLocation(const std::string file) const {
        // Remove leading /
        size_t firstNonSlash = file.find_first_not_of("/\\");
        std::string fileTrimmed = file.substr(firstNonSlash, file.size()-firstNonSlash);
        for(std::filesystem::path folder : _folders) {
            std::filesystem::path location = folder / std::filesystem::path(fileTrimmed);
            if(std::filesystem::exists(location)) {
                return location.string();
            }
        }
        throw std::runtime_error("[Util::Filemanager] Cannot retrieve the file location of file '" + file + "'");
    }

}
}