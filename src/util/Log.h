#ifndef ENGINE_UTILS_LOG_H
#define ENGINE_UTILS_LOG_H

#include <iostream>
#include <string>
#include <chrono>

namespace Engine {
namespace Util {
    void Log(std::string message);
    void Info(std::string message);
    void Error(std::string message);
    void Throw(std::string message);

    std::string GetTime();
}
}

#define LOG(message) Engine::Util::Log(message);
#define INFO(message) Engine::Util::Info(message);
#define WARNING(message) Engine::Util::Error(message);
#define THROW(message) { Engine::Util::Throw(message); throw std::runtime_error(message); }
#define ASSERT(check, message) if(!(check)) THROW(message)

#ifdef __DEBUG__
    #define ASSERT_IF_DEBUG(check, message) if(!(check)) THROW(message)
#else
    #define ASSERT_IF_DEBUG(check, message)
#endif

#endif