#ifndef ENGINE_NETWORK_WEBSOCKETROUTER_H
#define ENGINE_NETWORK_WEBSOCKETROUTER_H

#include "core/PCH.h"
#include "network/HTTP/Router.h"
#include "network/websocket/BasicHandler.h"
#include "network/WebsocketHandler.h"

namespace Engine {
namespace Network {

    /**
     * An utility class to make handeling websockets easier:
     * - It will automatically accept any incoming websocket request based on the AllowRequest() function
     * - It will bind this class to be the handeler of the websocket request if it is accepted
     */
    template<Util::Derived<Websocket::BasicHandler> Handler>
    class WebsocketRouter : private HTTP::Router, public Handler {
    public:

    private:
        
    };

}
}

#endif