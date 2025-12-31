#ifndef ENGINE_NETWORK_BASIC_WEBSOCKET_HANDLER_H
#define ENGINE_NETWORK_BASIC_WEBSOCKET_HANDLER_H

#include "core/PCH.h"
#include "network/websocket/Frame.h"
#include "network/HTTP/Request.h"

namespace Engine {
namespace Network {
    class WebsocketConnection;
    class WebHandler;
namespace Websocket {
    
    class BasicHandler {
    public:
        /**
         * Called just after the HTTP accept message is send.
         * Is called from the WebsocketHandler class.
         * 
         * @param connection The connection object on which the new connection started.
         * @param request The HTTP upgrade request.
         */
        virtual void OnWebsocketStart(WebsocketConnection& connection, HTTP::Request& request) {}
        /**
         * Is called from the WebsocketConnection class.
         * 
         * @param connection The connection object of which the connection stopped.
         */
        virtual void OnWebsocketStop(WebsocketConnection& connection) {}
        /**
         * Is called from the WebsocketConnection class.
         * 
         * @param connection The connection object on which a new message was received.
         * @param message The websocket message.
         */
        virtual void OnWebsocketMessage(WebsocketConnection& connection, Frame& message) {}
        /**
         * @brief Is called from the HTTP::Router to check if this handler supports a certain websocket protocol
         * See https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers/Sec-WebSocket-Protocol for more information about websocket protocols
         * 
         * @param protocol The protocol name the handler may support
         * @return If the handler supports the protocol
         */
        virtual bool SupportsProtocol(const std::string& protocol) { return false; }
    };

}
}
}

#endif