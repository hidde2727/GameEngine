#ifndef ENGINE_NETWORK_WEBHANDLER_H
#define ENGINE_NETWORK_WEBHANDLER_H

#include "core/PCH.h"

#include "network/HTTP/RequestHeader.h"
#include "network/HTTP/Response.h"
#include "network/websocket/Frame.h"

namespace Engine {
namespace Network {

    #define ENGINE_NETWORK_HTTPHANDLER_FUNCTION std::function<void(HTTP::RequestHeader& requestHeader, std::vector<uint8_t>& requestBody, HTTP::Response& response)>
    #define ENGINE_NETWORK_UPGRADEHANDLER_FUNCTION std::function<bool(HTTP::RequestHeader& requestHeader)>
    #define ENGINE_NETWORK_WEBSOCKETHANDLER_FUNCTION std::function<void(Websocket::Frame& frame, WebHandler::WebsocketConnection& connection)>
    #define ENGINE_NETWORK_ONWEBSOCKETSTART_FUNCTION std::function<void(WebHandler::WebsocketConnection& connection, const size_t uuid, HTTP::RequestHeader& requestHeader)>
    #define ENGINE_NETWORK_ONWEBSOCKETSTOP_FUNCTION std::function<void(WebHandler::WebsocketConnection& connection, const size_t uuid)>

    class WebHandler {
    public:
        class WebsocketConnection;
        WebHandler();

        void Start(
            ENGINE_NETWORK_HTTPHANDLER_FUNCTION httpHandler, 
            ENGINE_NETWORK_UPGRADEHANDLER_FUNCTION upgradeHandler, 
            ENGINE_NETWORK_WEBSOCKETHANDLER_FUNCTION websocketHandler,
            ENGINE_NETWORK_ONWEBSOCKETSTART_FUNCTION onWebsocketStart,
            ENGINE_NETWORK_ONWEBSOCKETSTOP_FUNCTION onWebsocketStop
        );
        void Start();
        // Will run the handeling functions on the thread the function is called
        // Makes sure no resources are accessed by the networking thread that are used by the main thread
        void HandleRequests();
        void Stop();

        std::string GetLocalAdress();

        void StopHTTPConnection(const size_t uuid);
        void StopWebsocketConnection(const size_t uuid);
        void UpgradeHTTPConnection(const size_t uuid, asio::ip::tcp::socket&& socket, HTTP::RequestHeader& requestHeader);

    private:
        friend class HTTPConnection;// Only acesses both io_context's and the handler functions
        friend class WebsocketConnection;// Only acesses both io_context's and the handler functions

        void AwaitConnection();

        // Used to make sure the request are handeled on the main thread
        asio::io_context _requestHandler;

        asio::io_context _context;
        std::thread _thread;
        std::string _localAddress;
        asio::ip::tcp::acceptor _acceptor;

        ENGINE_NETWORK_HTTPHANDLER_FUNCTION _httpHandler;
        ENGINE_NETWORK_UPGRADEHANDLER_FUNCTION _upgradeHandler;
        ENGINE_NETWORK_WEBSOCKETHANDLER_FUNCTION _websocketHandler;
        ENGINE_NETWORK_ONWEBSOCKETSTART_FUNCTION _onWebsocketStart;
        ENGINE_NETWORK_ONWEBSOCKETSTOP_FUNCTION _onWebsocketStop;

    public:
        class HTTPConnection : public std::enable_shared_from_this<HTTPConnection> {
        public:

            HTTPConnection(WebHandler* webhandler, asio::ip::tcp::socket&& socket);
            ~HTTPConnection();

            void Start(const size_t uuid);
            void Stop();

        private:
            void ReceiveHeader();
            void ReceiveBody(const size_t size);
            void Write();
            void HandleRequest();
            void UpgradeConnection();
            void ScheduleTimeoutCheck();

            WebHandler* _webhandler;
            asio::ip::tcp::socket _socket;
            asio::system_timer _timeout;
            size_t _uuid;

            asio::streambuf _inputBuffer;
            std::shared_ptr<HTTP::RequestHeader> _header;
            std::shared_ptr<std::vector<uint8_t>> _body;

            std::queue<std::shared_ptr<HTTP::Response>> _writeQueue;
            bool _connectionKeepAlive = false;
            bool _isStopped = false;
        };
    private:
        std::map<size_t, std::shared_ptr<HTTPConnection>> _httpConnections;
    public:
        class WebsocketConnection : public std::enable_shared_from_this<WebsocketConnection> {
        public:

            WebsocketConnection(WebHandler* webhandler, asio::ip::tcp::socket&& socket);
            ~WebsocketConnection();

            void Start(const size_t uuid);
            void Stop();

            // Safe to call from the main thread
            void SendData(std::shared_ptr<Websocket::Frame> data);

        private:
            void ReceiveFirstPartHeader();
            void ReceiveSecondPartHeader();
            void ReceiveBody();
            void HandleReceive();
            void Write();
            void StopWithoutHandshake();

            WebHandler* _webhandler;
            asio::ip::tcp::socket _socket;
            asio::system_timer _timeout;// The timeout is only used to make sure we close after a closing frame
            size_t _uuid;

            asio::streambuf _inputBuffer;
            
            bool _isStopped = false;

            std::shared_ptr<Websocket::Frame> _receivingFrame;
            bool _closeAfterWrite = false;
            bool _closeAfterRead = false;
            std::queue<std::shared_ptr<Websocket::Frame>> _writeQueue;
        };
    private:
        std::map<size_t, std::shared_ptr<WebsocketConnection>> _websocketConnections;

    };

}
}

#endif