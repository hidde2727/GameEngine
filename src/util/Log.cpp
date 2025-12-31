#include "util/Log.h"
#include "util/serialization/Serialization.h"

namespace Engine {
namespace Util {

    void Log(std::string message) {
        std::cout << GetTime() << SetLogColor() << message << ResetLogColor() << "\n";;
    }
    void Info(std::string message) {
        std::cout << GetTime() << SetInfoColor() << message << ResetLogColor() << "\n";;
    }
    void Error(std::string message) {
        std::cout << GetTime() << SetErrorColor() << message << ResetLogColor() << "\n";;
    }
    void Throw(std::string message) {
        std::cout << GetTime() << SetThrowColor() << message << ResetLogColor() << "\n";
    }

    constexpr std::string SetLogColor() {
        return "\033[0m";
    }
    constexpr std::string SetInfoColor() {
        return "\033[34m";
    }
    constexpr std::string SetErrorColor() {
        return "\033[31m";
    }
    constexpr std::string SetThrowColor() {
        return "\033[91m";
    }
    constexpr std::string ResetLogColor() {
        return "\033[0m";
    }

    std::string GetTime() {
		auto now = std::chrono::system_clock::now();
		auto nowMS = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		auto time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;

        ss << '[';
		ss << std::put_time(std::localtime(&time_t), "%H:%M:%S:");
		ss << std::setw(3) << std::setfill('0') << nowMS % 1000;
        ss << "] ";

        return ss.str();
	}
    
    
}
}