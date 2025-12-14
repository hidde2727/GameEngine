#ifndef ENGINE_NETWORK_SESSIONSTORAGE_H
#define ENGINE_NETWORK_SESSIONSTORAGE_H

#include "core/PCH.h"
#include "util/serialization/Serialization.h"

namespace Engine {
namespace Network {

    #define ENGINE_NETWORK_SESSION_EXPIRATION std::chrono::days(30)
    class SessionStorage : public Util::Serializable {
    public:

        struct Session {
            std::map<std::string, std::string> _session;
            std::chrono::steady_clock::time_point _start;
            uint64_t _id;
        };

        uint64_t NewSession();
        /**
         * @brief Try to get a session
         * @param id The id to get
         * @return Eiter the session, or nullptr if not found
         */
        Session* GetSession(uint64_t id);

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

        void Serialize(Util::Serializer* s) override {
            s->Serialize(_sessions);
        }
        void Deserialize(Util::Deserializer* d) override {
            d->Deserialize(_sessions);
        }

    private:
        std::map<uint64_t, Session> _sessions;
    };

}
}

#endif