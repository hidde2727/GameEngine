#ifndef ENGINE_NETWORK_HTTP_SESSIONSTORAGE_H
#define ENGINE_NETWORK_HTTP_SESSIONSTORAGE_H

#include "core/PCH.h"
#include "util/serialization/Serialization.h"
#include "util/serialization/Binary.h"

namespace Engine {
namespace Network {
namespace HTTP {

    #ifndef ENGINE_NETWORK_SESSION_EXPIRATION
    #define ENGINE_NETWORK_SESSION_EXPIRATION std::chrono::days(30)
    #endif
    #ifndef ENGINE_NETWORK_SESSION_COOKIENAME
    #define ENGINE_NETWORK_SESSION_COOKIENAME(id) std::string("GameEngine-Session-") + std::to_string(id)
    #endif

    struct Session {
        std::map<std::string, std::string> _session;
        std::chrono::steady_clock::time_point _start;
        uint64_t _id;
    };

    class SessionStorage {
    public:

        /**
         * @brief Create a session storage, the ID must be unique between all the storages
         * 
         * @param id The ID to use in the cookie name sent to every client
         */
        SessionStorage(const uint64_t id);
        /**
         * @brief Returns the name to use in a cookie for storing the session ID
         * 
         * @return std::string The name 
         */
        std::string GetCookieName() { return _cookieID; }

        uint64_t CreateSession();
        bool HasSession(const uint64_t id);
        void RemoveSession(const uint64_t id);
        /**
         * @brief Try to get a session
         * @param id The id to get
         * @return Eiter the session, or nullptr if not found
         */
        std::shared_ptr<Session> GetSession(const uint64_t id);

        /**
         * @brief If session exists, cals GetSession(), else creates a session and returns it
         * 
         * @param id The id to try and get, if it doesn't exist this id gets set to a new one
         * @return The session
         */
        std::shared_ptr<Session> GetOrCreateSession(uint64_t& id);

        /**
         * @brief Stores the sessions in the cache
         * @param cacheID cacheID
         */
        void StoreToCache(const std::string cacheID);
        /**
         * @brief Tries to load the sessions from the cache
         * @param cacheID the cacheID to try and load
         */
        void LoadFromCache(const std::string cacheID);

    private:
        friend class Util::Serializer;
        friend class Util::Deserializer;

        std::map<uint64_t, std::shared_ptr<Session>> _sessions;
        [[=Util::SkipSerialization]] std::string _cookieID;
    };

}
}
}

#endif