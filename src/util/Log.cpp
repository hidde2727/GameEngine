#include "util/Log.h"

namespace Engine {
namespace Util {

    void Log(std::string message) {
        std::cout << GetTime() << message << '\n';
    }
    void Info(std::string message) {
        std::cout << GetTime()  << "\033[34m" << message << "\033[0m\n";
    }
    void Error(std::string message) {
        std::cout << GetTime()  << "\033[31m" << message << "\033[0m\n";
    }
    void Throw(std::string message) {
        std::cout << GetTime()  << "\033[91m" << message << "\033[0m\n";
    }

    std::string GetTime() {
		auto now = std::chrono::system_clock::now();
		auto nowMS = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		auto time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;

		ss << std::setw(3) << std::setfill('0') << nowMS % 1000;
		std::string strNowMS = ss.str();
		ss.str("");
		ss << std::put_time(std::localtime(&time_t), "%H:%M:%S:") << strNowMS;

		return '['+ss.str()+"] ";
	}

}
}