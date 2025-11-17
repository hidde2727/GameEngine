#ifndef ENGINE_NETWORK_WEBSOCKETHANDLER_H
#define ENGINE_NETWORK_WEBSOCKETHANDLER_H

#include "core/PCH.h"
#include "network/websocket/Frame.h"
#include "network/HTTP/Request.h"

namespace Engine {
namespace Network {

    class WebsocketConnection;
    class WebsocketHandler {
    public:
        /**
         * Called just after the HTTP accept message is send.
         * Is called from the WebsocketHandler class.
         * 
         * @param connection The connection object on which the new connection started.
         * @param request The HTTP upgrade request.
         */
        virtual void OnWebsocketStart(WebsocketConnection* connection, HTTP::Request& request) {}
        /**
         * Is called from the WebsocketConnection class.
         * 
         * @param connection The connection object of which the connection stopped.
         */
        virtual void OnWebsocketStop(WebsocketConnection* connection) {}
        /**
         * Is called from the WebsocketConnection class.
         * 
         * @param connection The connection object on which a new message was received.
         * @param message The websocket message.
         */
        virtual void OnWebsocketMessage(WebsocketConnection* connection, Websocket::Frame& message) {}
    private:
        // friend class WebsocketConnection;
        // void OnWebsocketStartInternal(WebsocketConnection* connection, HTTP::Request& request);
        // void OnWebsocketStopInternal(WebsocketConnection* connection);
        // void OnWebsocketMessageInternal(WebsocketConnection* connection, Websocket::Frame& message);
    };

}
}

#endif