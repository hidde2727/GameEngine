#include "util/FileManager.h"

namespace Engine {
namespace Util {

    bool File::Exists() const {
        return std::filesystem::exists(_path);
    }
    bool File::IsRegular() const {
        return std::filesystem::is_regular_file(_path);
    }
    bool File::IsDirectory() const {
        return std::filesystem::is_directory(_path);
    }
    bool File::IsImage() const {
        if(!IsRegular()) return false;
        int width, height, comp;
        return GetImageInfo(width, height, comp);
    }
    std::filesystem::file_time_type File::LastChange() const {
        ASSERT(Exists(), "[Util::File] Cannot query the last change of a file that does not exist")
        return std::filesystem::last_write_time(_path);
    }
    size_t File::GetSize() const {
        ASSERT(Exists(), "[Util::File] Cannot query the size of a file that does not exist")
        return std::filesystem::file_size(_path);
    }

    std::ifstream File::GetInStream(const std::ios_base::openmode inputMode) const {
        ASSERT(Exists(), "[Util::File] Cannot get the inputstream of a file that does not exist")
        std::ifstream inputStream(_path, inputMode);
        ASSERT(!inputStream.fail(), ("[Util::Filemanager] Failed to open input stream for file '" + _path.string() + "' because : \n\t" + std::string(strerror(errno))).c_str());
        return inputStream;
    }
    void File::Read(std::string& readBuffer, const std::ios_base::openmode inputMode) const {
        ASSERT(Exists(), "[Util::File] Cannot read in a file that does not exist")
        readBuffer.resize(GetSize());
        Read(readBuffer.data(), readBuffer.size(), inputMode);
    }

    void File::Create() const {
        GetOutStream().close();
    }
    std::ofstream File::GetOutStream(const std::ios_base::openmode outputMode) const {
        std::ofstream outputStream(_path, outputMode);
        ASSERT(!outputStream.fail(), ("[Util::Filemanager] Failed to open output stream for file '" + _path.string() + "' because : \n\t" + std::string(strerror(errno))).c_str());
        return outputStream;
    }
    void File::Write(std::string& buffer, const std::ios_base::openmode outputMode) const {
        Write(buffer.data(), buffer.size(), outputMode);
    }

    bool File::GetImageInfo(int& width, int& height, int& comp) const {
        return stbi_info(_path.c_str(), &width, &height, &comp);
    }
    std::shared_ptr<uint8_t> File::ReadImage(int& width, int& height, int& channelsInFile, int desiredChannels) const {
        ASSERT(IsRegular(), "[Util::File] Cannot read in an image on anything other than a regular file")
        uint8_t* data = stbi_load(_path.c_str(), &width, &height, &channelsInFile, desiredChannels);
        ASSERT(data!=NULL, "[Util::File] Failed to read image because: '" + GetImageReadingError() + "'")
        return std::shared_ptr<uint8_t>(data, stbi_image_free);
    }
    std::string File::GetImageReadingError() const {
        return std::string(stbi_failure_reason());
    }

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

    File FileManager::Cache(const CacheID cacheID) {
        std::string cacheFile = Util::Base64FileEncode(Util::SHA1(cacheID)) + std::filesystem::path(cacheID).extension().string();
        return File((_cacheFolder / std::filesystem::path(cacheFile)).string());
    }
    File FileManager::Get(const std::string file) {
        // Remove leading /
        size_t firstNonSlash = file.find_first_not_of("/\\");
        std::string fileTrimmed = file.substr(firstNonSlash, file.size()-firstNonSlash);
        for(std::filesystem::path searchPath : _searchPaths) {
            std::filesystem::path location = searchPath / std::filesystem::path(fileTrimmed);
            if(std::filesystem::exists(location)) {
                return File(location.string());
            }
        }
        throw std::runtime_error("[Util::Filemanager] Cannot retrieve the file location of file '" + file + "'");
    }

    bool FileManager::CanUseCache(const CacheID cacheID, const std::initializer_list<std::string> cacheFileDependency) {
        File cacheFile = Cache(cacheID);
        if(!cacheFile.Exists()) return false;
        std::filesystem::file_time_type lastCacheChange = cacheFile.LastChange();
        for(std::string filePath : cacheFileDependency) {
            File file("");
            try {
                file = Get(filePath);
            } catch(...) {
                THROW("[Util::Filemanager] Specified cache dependency does not exist")
            }
            std::filesystem::file_time_type lastFileChange = file.LastChange();
            if(lastFileChange > lastCacheChange) return false;
        }
        return true;
    }

}
}