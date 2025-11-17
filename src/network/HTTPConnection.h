#ifndef ENGINE_NETWORK_HTTPCONNECTION_H
#define ENGINE_NETWORK_HTTPCONNECTION_H

#include "core/PCH.h"
#include "network/HTTP/Request.h"
#include "network/HTTP/Response.h"

namespace Engine {
namespace Network {

    class WebHandler;
    class HTTPConnection : public std::enable_shared_from_this<HTTPConnection> {
    public:

        HTTPConnection(std::weak_ptr<WebHandler> webhandler, asio::ip::tcp::socket&& socket);
        ~HTTPConnection();

        void Start(const size_t uuid);
        void Stop();

    private:
        void ReceiveHeader();
        void ReceiveBody(const size_t size);
        void Write();
        void HandleRequest();
        void ScheduleTimeoutCheck();

        std::weak_ptr<WebHandler> _webhandler;
        asio::ip::tcp::socket _socket;
        asio::system_timer _timeout;
        size_t _uuid;

        asio::streambuf _inputBuffer;
        std::shared_ptr<HTTP::Request> _request;

        std::queue<std::shared_ptr<HTTP::Response>> _writeQueue;
        bool _connectionKeepAlive = false;
        bool _isStopped = false;
    };

}
}

#endif