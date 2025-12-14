#include "network/SessionStorage.h"
#include "util/serialization/Binary.h"
#include "util/FileManager.h"

namespace Engine {
namespace Network {

    std::random_device r;
    std::default_random_engine randomEngine(r());
    std::uniform_int_distribution<uint64_t> uniform_dist(0, UINT64_MAX);

    uint64_t SessionStorage::NewSession() {
        uint64_t id = 0;
        do {
            id = randomEngine();
        } while(_sessions.contains(id));
        _sessions[id] = Session();
    }
    SessionStorage::Session* SessionStorage::GetSession(uint64_t id) {
        if(!_sessions.contains(id)) return nullptr;
        if(_sessions[id]._start + ENGINE_NETWORK_SESSION_EXPIRATION > std::chrono::steady_clock::now()) {
            _sessions.erase(id);
            return nullptr;
        }
        return &_sessions[id];
    }
    void SessionStorage::StoreToCache(const std::string cacheID) {
        Util::BinarySerializer serializer;
        std::ofstream out = Util::FileManager::AddCachedFile(cacheID, std::ios_base::binary);
        serializer.Serialize(*this, out);
    }
    void SessionStorage::LoadFromCache(const std::string cacheID) {
        Util::BinaryDeserializer deserializer;
        std::ifstream in = Util::FileManager::GetCachedFile(cacheID, std::ios_base::binary);
        deserializer.Deserialize(*this, in);
    }

}
}