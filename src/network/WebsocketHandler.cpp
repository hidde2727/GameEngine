#include "network/WebsocketHandler.h"

namespace Engine {
namespace Network {

    void WebsocketHandler::OnWebsocketStartInternal(WebsocketConnection* connection, HTTP::Request& request) {
        OnWebsocketStart(connection, request);
    }
    void WebsocketHandler::OnWebsocketStopInternal(WebsocketConnection* connection) {
        OnWebsocketStop(connection);
    }
    void WebsocketHandler::OnWebsocketMessageInternal(WebsocketConnection* connection, Websocket::Frame& message) {
        OnWebsocketMessage(connection, message);
    }

}
}