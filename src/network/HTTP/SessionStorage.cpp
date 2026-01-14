#include "network/HTTP/SessionStorage.h"
#include "util/serialization/Binary.h"
#include "util/FileManager.h"

namespace Engine {
namespace Network {
namespace HTTP {

    SessionStorage::SessionStorage(const uint64_t id) {
        _cookieID = ENGINE_NETWORK_SESSION_COOKIENAME(id);
    }

    std::random_device r;
    std::default_random_engine randomEngine(r());
    std::uniform_int_distribution<uint64_t> uniform_dist(0, UINT64_MAX);

    uint64_t SessionStorage::CreateSession() {
        uint64_t id = 0;
        do {
            id = randomEngine();
        } while(_sessions.contains(id));
        _sessions[id] = std::make_shared<Session>();
        _sessions[id]->_start = std::chrono::steady_clock::now();
        return id;
    }
    bool SessionStorage::HasSession(const uint64_t id) {
        if(!_sessions.contains(id)) return false;
        if(_sessions[id]->_start + ENGINE_NETWORK_SESSION_EXPIRATION < std::chrono::steady_clock::now()) {
            _sessions.erase(id);
            return false;
        }
        return true;
    }
    void SessionStorage::RemoveSession(const uint64_t id) {
        _sessions.erase(id);
    }
    std::shared_ptr<Session> SessionStorage::GetSession(const uint64_t id) {
        if(!HasSession(id)) return nullptr;
        return _sessions[id];
    }
    std::shared_ptr<Session> SessionStorage::GetOrCreateSession(uint64_t& id) {
        if(HasSession(id)) return _sessions[id];
        id = CreateSession();
        return _sessions[id];
    }
    void SessionStorage::StoreToCache(const std::string cacheID) {
        Util::BinarySerializer serializer;
        std::ofstream out = Util::FileManager::Cache(cacheID).GetOutStream(std::ios::binary | std::ios::trunc);
        serializer.Serialize(*this, out, Util::BinarySerializationOutputFlag::IncludeTypeInfo);
    }
    void SessionStorage::LoadFromCache(const std::string cacheID) {
        try {
            if(!Util::FileManager::Cache(cacheID).Exists()) return;
            Util::BinaryDeserializer deserializer;
            std::ifstream in = Util::FileManager::Cache(cacheID).GetInStream(std::ios::binary);
            deserializer.Deserialize(*this, in);
        } catch(...) {
            WARNING("[HTTP::SessionStorage] Failed to load previous sessions from the cache")
        }
    }

}
}
}