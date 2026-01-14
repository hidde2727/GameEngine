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
     * - It will automatically accept any incoming websocket requests
     * - It will bind this class to be the handeler of the websocket request if it is accepted
     */
    class WebsocketRouter : private HTTP::Router, public WebsocketHandler<WebsocketRouter> {
    public:
        std::shared_ptr<Network::HTTP::Response> HandleRequest(Network::HTTP::Request& request) override {
            if(Get("IsWebsocketAlive")) return Text("true");
            if(WebsocketUpgrade()) return AcceptWebsocket(this);
            return NotHandled();
        }
    };

}
}

#endif