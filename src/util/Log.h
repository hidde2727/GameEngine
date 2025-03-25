#include <iostream>
#include <string>
#include <chrono>

namespace Engine {
namespace Util{
    void Log(std::string message);
    void Info(std::string message);
    void Error(std::string message);
    void Throw(std::string message);

    std::string GetTime();
}
}

#define LOG(message) Engine::Util::Log(message);
#define INFO(message) Engine::Util::Info(message);
#define ERROR(message) Engine::Util::Error(message);
#define THROW(message) { Engine::Util::Throw(message); throw new std::runtime_error(message); }
#define ASSERT(check, message) if(check) THROW(message)