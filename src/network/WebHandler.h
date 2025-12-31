#ifndef ENGINE_NETWORK_WEBHANDLER_H
#define ENGINE_NETWORK_WEBHANDLER_H

#include "core/PCH.h"

#include "network/HTTP/Request.h"
#include "network/HTTP/Response.h"
#include "network/HTTP/Connection.h"
#include "network/HTTP/Router.h"

#include "network/websocket/Frame.h"
#include "network/websocket/Connection.h"
#include "network/websocket/BasicHandler.h"

#ifndef ENGINE_NETWORK_LAN_POLLING_RATE
#define ENGINE_NETWORK_LAN_POLLING_RATE std::chrono::seconds(3)
#endif

#ifndef ENGINE_NETWORK_VERBOSE_HTTP
#define ENGINE_NETWORK_VERBOSE_HTTP false
#endif

#ifndef ENGINE_NETWORK_VERBOSE_WEBSOCKET
#define ENGINE_NETWORK_VERBOSE_WEBSOCKET false
#endif

#define ENGINE_NETWORK_VERBOSE_HTTP_WEBSOCKET ENGINE_NETWORK_VERBOSE_HTTP || ENGINE_NETWORK_VERBOSE_WEBSOCKET

namespace Engine {
namespace Network {

    /// Create using the WebHandler::Create() function
    class WebHandler : public std::enable_shared_from_this<WebHandler>, public HTTP::Router {
    private:
        struct Private{ explicit Private() = default; };
    public:

        /// Is private to make sure you can only create this as a shared_ptr
        WebHandler(Private);

        /// Factory function to force shared_ptr usage
        static std::shared_ptr<WebHandler> Create() {
            return std::make_shared<WebHandler>(Private());
        }

        void Start();
        /**
         * Used for thread safety, should be called on the main thread.
         * This functions makes it possible for the WebHandler to execute code on the main thread (and not the network thread).
         * This makes it possible to access resources not accessible in the networking thread.
         */
        void Update();
        void Stop();

        std::string GetLocalAdress();

    private:
        friend class HTTPConnection;/// Acesses both io_context's and Router class
        friend class WebsocketConnection;/// Acesses both io_context's

        /// Used to expose the same method from Router to the HTTPConnection class
        inline std::shared_ptr<HTTP::Response> HandleRequestInternal(std::shared_ptr<HTTP::Request> request) {
            return Router::HandleRequestInternal(request);
        }

        /// Used by the HTTPConnection class
        void StopHTTPConnection(const size_t uuid);
        /// Used by the WebsocketConnection class
        void StopWebsocketConnection(const size_t uuid);
        /// Used by the HTTPConnection class
        void UpgradeHTTPConnection(const size_t uuid, asio::ip::tcp::socket&& socket, std::shared_ptr<HTTP::Request> request, std::shared_ptr<HTTP::Response> response);

        void AwaitConnection();

        /// Used to make sure the request are handeled on the main thread
        asio::io_context _requestHandler;

        asio::io_context _context;
        std::thread _thread;
        std::string _localAddress;
        asio::ip::tcp::acceptor _acceptor;
        bool _running = false;

        std::map<size_t, std::shared_ptr<HTTPConnection>> _httpConnections;
        std::map<size_t, std::shared_ptr<WebsocketConnection>> _websocketConnections;

    };

}
}

#endif