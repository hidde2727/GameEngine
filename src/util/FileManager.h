#ifndef ENGINE_UTIL_RESOURCE_MANAGER_H
#define ENGINE_UTIL_RESOURCE_MANAGER_H

#include "core/PCH.h"
#include "util/Hashing.h"

namespace Engine {
namespace Util {

    typedef std::string CacheID;

    /**
     * @brief A class for easy file access
     * The file may not exist on the file system
     * 
     */
    class File {
    public:

        /**
         * @brief Construct a File
         * 
         * @param _path The path this class will operate on 
         */
        File(const std::filesystem::path path) : _path(path) {}

        /**
         * @brief Check is the file exists
         * 
         * @return bool
         */
        bool Exists() const;
        /**
         * @brief Check if it is an file (will return false if it is for example a folder)
         * 
         * @return bool
         */
        bool IsRegular() const;
        /**
         * @brief Check if the location is a folder
         * 
         * @return bool
         */
        bool IsDirectory() const;
        /**
         * @brief Check if it is an image file
         * Is the same as:
         * ```
         * int width, height, comp;
         * bool isImage = GetImageInfo(width, height, comp);
         * ```
         * 
         * @return bool
         */
        bool IsImage() const;
        /**
         * @brief Returns the last moment the file content was changed
         * 
         * @return std::chrono::time_point
         */
        std::filesystem::file_time_type LastChange() const;
        /**
         * @brief Will return the size of the file
         * 
         * @return bool
         */
        size_t GetSize() const;
        /**
         * @brief Returns the input stream if the file exists, else it throws
         * 
         * @param inputMode The io mode to open the file in (for example std::ios::trunc to override the existing content)
         * @return std::ifstream
         */
        std::ifstream GetInStream(const std::ios_base::openmode inputMode = 0) const;
        template<class T>
        void Read(T* readBuffer, const size_t readBufferSize, const std::ios_base::openmode inputMode = std::ios_base::binary) const;
        template<class T>
        void Read(std::vector<T>& readBuffer, const std::ios_base::openmode inputMode = std::ios_base::binary) const;
        void Read(std::string& readBuffer, const std::ios_base::openmode inputMode = 0) const;

        /**
         * @brief Creates the file, if it does not exist on the file system
         * Is the same as calling:
         * ```
         * GetOutStream().close();
         * ```
         */
        void Create() const;
        /**
         * @brief Optionally creates the file, then returns the output stream
         * 
         * @param outputMode The ios mode to open the stream in (for example std::ios::trunc to override the existing content)
         * @return std::ofstream
         */
        std::ofstream GetOutStream(const std::ios_base::openmode outputMode = 0) const;
        template<class T>
        void Write(T* buffer, const size_t bufferSize, const std::ios_base::openmode outputMode = std::ios_base::binary) const;
        template<class T>
        void Write(std::vector<T>& buffer, const std::ios_base::openmode outputMode = std::ios_base::binary) const;
        void Write(std::string& buffer, const std::ios_base::openmode outputMode = 0) const;

        /**
         * @brief Get the info of an image
         * 
         * @param width The width of the image
         * @param height The height of the image 
         * @param comp The amount of components (3 components implies the image is in RGB, 4 implies RGBA)
         * @return If it is an success or not
         */
        bool GetImageInfo(int& width, int& height, int& comp) const;
        /**
         * @brief Read the image from the file
         * 
         * @param width The width of the image
         * @param height The height of the image
         * @param channelsInFile The amount of channels originally in the file, if desiredChannels was ignored
         * @param desiredChannels The amount of channels to read in
         * @return A pointer to the image data, that automatically deletes the data when it is not used anymore
         */
        std::shared_ptr<uint8_t> ReadImage(int& width, int& height, int& channelsInFile, int desiredChannels=3) const;
        /**
         * @brief If the process of getting the info of an image fails, you can use this function to get the reason for the failure
         * 
         * @return The error
         */
        std::string GetImageReadingError() const;

        inline std::string String() const {
            return _path.string();
        }

        inline explicit operator std::string() const {
            return String();
        }

    private:
        std::filesystem::path _path;
    };

    /** 
     * Is created with a prioritized list of searchPaths, 
     * When you are trying to get a file it will return the file from the first searchPath that contains the file.
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
         * @brief Returns a location in the cache folder
         * The resulting location in the cache folder is Base64(SHA256(cacheID)) + fileExtension
         * 
         * @param cacheID Should be an identifier ending with a file extension (using the relative path of the original file as cacheID is recommended)
         * @return File
         */
        static File Cache(const CacheID cacheID);
        /**
         * @brief Retrieves an existing file on the disk, it must exist in one of the search directories
         * 
         * @param path The path to search for in the search directories
         * @return File
         * @throws If path is unfindable in all the search directories
         */
        static File Get(const std::string path);
    
        /// Will check if one of the inputFilePaths changed after the cacheID was created
        static bool CanUseCache(const CacheID cacheID, const std::initializer_list<std::string> inputFiles);

    private:

        static std::vector<std::filesystem::path> _searchPaths;
        static std::filesystem::path _cacheFolder;
    };





    template<class T>
    void File::Read(T* buffer, const size_t bufferSize, const std::ios_base::openmode inputMode) const {
        std::ifstream inputStream = GetInStream(inputMode);
        inputStream.read(reinterpret_cast<char*>(buffer), bufferSize*sizeof(T));
    }
    template<class T>
    void File::Read(std::vector<T>& buffer, const std::ios_base::openmode inputMode) const {
        buffer.resize(ceil( GetSize()/(float)sizeof(T)));
        Read(buffer.data(), buffer.size(), inputMode);
    }

    template<class T>
    void File::Write(T* buffer, const size_t bufferSize, const std::ios_base::openmode outputMode) const {
        std::ofstream outputStream = GetOutStream(outputMode);
        outputStream.write(reinterpret_cast<char*>(buffer), bufferSize*sizeof(T));
    }
    template<class T>
    void File::Write(std::vector<T>& buffer, const std::ios_base::openmode outputMode) const {
        Write(buffer.data(), buffer.size(), outputMode);
    }



    

}
}

#endif