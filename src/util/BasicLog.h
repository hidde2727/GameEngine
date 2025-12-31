#ifndef ENGINE_UTIL_BASICLOG_H
#define ENGINE_UTIL_BASICLOG_H

#include <iostream>
#include <string>
#include <chrono>

namespace Engine {
namespace Util {
    
    /**
     * @brief Log a string message, with nice formatting and the LOG color
     * 
     * @param message The message to log 
     */
    void Log(std::string message);
    /**
     * @brief Log a string message, with nice formatting and the INFO color
     * 
     * @param message The message to log 
     */
    void Info(std::string message);
    /**
     * @brief Log a string message, with nice formatting and the ERROR color
     * 
     * @param message The message to log 
     */
    void Error(std::string message);
    /**
     * @brief Log a string message, with nice formatting and the THROW color
     * 
     * @param message The message to log 
     */
    void Throw(std::string message);

    /**
     * @brief Returns the string to set the terminal to the LOG color
     * 
     * @return The string to set the color
     */
    constexpr std::string SetLogColor();
    /**
     * @brief Returns the string to set the terminal to the INFO color
     * 
     * @return The string to set the color
     */
    constexpr std::string SetInfoColor();
    /**
     * @brief Returns the string to set the terminal to the ERROR color
     * 
     * @return The string to set the color
     */
    constexpr std::string SetErrorColor();
    /**
     * @brief Returns the string to set the terminal to the THROW color
     * 
     * @return The string to set the color 
     */
    constexpr std::string SetThrowColor();
    /**
     * @brief Returns the string to reset the terminal color
     * 
     * @return The string to reset the terminal color 
     */
    constexpr std::string ResetLogColor();

    /**
     * @brief Returns a nicely formatted time string
     * Example: '[00:00:00]
     *  
     * @return The time string 
     */
    std::string GetTime();
}
}

#define LOG(message) Engine::Util::Log(message);
#define INFO(message) Engine::Util::Info(message);
#define WARNING(message) Engine::Util::Error(message);
#define THROW(message) { Engine::Util::Throw(message); throw std::runtime_error(message); }
#define ASSERT(check, message) if(!(check)) THROW(message)

#ifdef __DEBUG__
#   define ASSERT_IF_DEBUG(check, message) if(!(check)) THROW(message)
#else
#   define ASSERT_IF_DEBUG(check, message)
#endif

#endif