#ifndef ENGINE_CORE_SYSTEM_H
#define ENGINE_CORE_SYSTEM_H

#include "core/PCH.h"

class SystemInternal {
public:

private:
    virtual void Update(entt::registry registry, const float dt);
};

// Wrapper class for easy entt view usage
// Usage, creating a class that inherits with as template arguments the components the system uses
//        then overriding the UpdateEntity function
template<class... Ts>
class System : public SystemInternal{
public:
    inline virtual void UpdateEntity(const float dt, const entt::entity, Ts...) = 0;
private:
    friend class Scene;
    
    void Update(entt::registry registry, const float dt) override {
        auto view = registry.view<Ts>();
        for(const entt::entity entity : view) {
            UpdateEntity(dt, entity, view.get<Ts>(entity));
        }
    }
};

#endif