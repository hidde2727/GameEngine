#include "network/HTTP/Connection.h"
#include "network/WebHandler.h"

#include "util/Hashing.h"

namespace Engine {
namespace Network {

    HTTPConnection::HTTPConnection(std::weak_ptr<WebHandler> webhandler, asio::ip::tcp::socket&& socket) :
    _webhandler(webhandler),
    _socket(std::move(socket)),
    _timeout(webhandler.lock()->_context)
    {
    }
    HTTPConnection::~HTTPConnection() {
        if(!_isStopped) Stop();
    }
    
    void HTTPConnection::Start(const size_t uuid) {
        _uuid = uuid;
        ScheduleTimeoutCheck();
        ReceiveHeader();
    }
    void HTTPConnection::Stop() {
        if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Stopping connection (" + std::to_string(_uuid) + ")")
        if(_isStopped) return;
        _isStopped = true;
        if(_socket.is_open()) _socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        _timeout.cancel();
        _webhandler.lock()->StopHTTPConnection(_uuid);
        if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Stopped connection (" + std::to_string(_uuid) + ")")
    }
    void HTTPConnection::ReceiveHeader() {
        ScheduleTimeoutCheck();
        std::shared_ptr<HTTPConnection> self = shared_from_this();

        asio::async_read_until(_socket, _inputBuffer, "\r\n\r\n",
        [this, self](const std::error_code& ec, std::size_t bytesTransferred)
        {
            if (ec) {
				if(ec!=asio::error::eof) WARNING("Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
			}
            
            std::istream is(&_inputBuffer);
            _request = std::make_shared<HTTP::Request>();
            _request->ParseHeader(is, bytesTransferred);
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Connection (" + std::to_string(_uuid) + ") received header: " + _request->GetHead())

            std::string contentLength = _request->GetHeader("Content-Length");
            if(contentLength.size()>0) {
                ReceiveBody(stoi(contentLength));
                return;
            }
            // There is no body, so handle the request
            HandleRequest();
            ReceiveHeader();
        }
        );
    }
    void HTTPConnection::ReceiveBody(const size_t size) {
        ScheduleTimeoutCheck();
        std::shared_ptr<HTTPConnection> self = shared_from_this();

        _request->GetBody().resize(size);

        asio::async_read(_socket, asio::buffer(_request->GetBody().data(), _request->GetBody().size()),
        [this, self](const std::error_code& ec, std::size_t bytesTransferred)
        {
            if (ec) {
				if(ec!=asio::error::eof) WARNING("[HTTPConnection] Connection (" + std::to_string(_uuid) + ") stopped because: " + ec.message())
				Stop();
				return;
			}
            
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Received body on connection (" + std::to_string(_uuid) + ") with head (" + _request->GetHead() + ")")

            HandleRequest();
            ReceiveHeader();
        }
        );
    }
    void HTTPConnection::Write() {
		if (_writeQueue.empty()) return;
        ScheduleTimeoutCheck();

        std::shared_ptr<HTTPConnection> self = shared_from_this();

        asio::async_write(_socket, _writeQueue.front()->ToBuffers(),
        [this, self](const std::error_code& ec, std::size_t bytesTransferred)
        {
            if (ec) {
				if(ec!=asio::error::eof) WARNING("[HTTPConnection] Connection (" + std::to_string(_uuid) + ") stopped because: " + ec.message())
				Stop();
				return;
            }
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Response for connection (" + std::to_string(_uuid) + ") send")
            _writeQueue.pop();

            if(!_connectionKeepAlive) return Stop();

            if (!_writeQueue.empty()) Write();
        });
    }

    void HTTPConnection::HandleRequest() {
        std::shared_ptr<HTTPConnection> self = shared_from_this();

        std::shared_ptr<HTTP::Request> request = _request;
        // Post work for the main thread
        asio::post(asio::bind_executor(_webhandler.lock()->_requestHandler, 
        [this, self, request]() {
            std::shared_ptr<HTTP::Response> response = _webhandler.lock()->HandleRequestInternal(request);
            if(response == nullptr) response = _webhandler.lock()->NotFound(request);

            if(response->GetHeader("Connection") == "Keep-Alive" || response->IsWebsocketUpgrade()) {
                _connectionKeepAlive = true;
            }
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Response for connection (" + std::to_string(_uuid) + ") with request (" + request->GetHead() + ") created")

            // Post work for the networking thread
            asio::post(_webhandler.lock()->_context, [this, self, request, response]() {
                bool isUpgradeRequest = response->IsWebsocketUpgrade();
                _writeQueue.push(response);
                if(_writeQueue.size() == 1) Write();
                if(!isUpgradeRequest) return;
                else {
                    _isStopped = true;
                    _webhandler.lock()->UpgradeHTTPConnection(_uuid, std::move(_socket), request, response);
                }
            });
        }));
    }

    void HTTPConnection::ScheduleTimeoutCheck() {
        return;
        std::shared_ptr<HTTPConnection> self = shared_from_this();
        
        _timeout.expires_after(std::chrono::seconds(5));
        _timeout.async_wait([this, self](const std::error_code& e) {
            if(e == asio::error::operation_aborted) return;
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Stopping connection (" + std::to_string(_uuid) + ") because of timeout")
            Stop();
        });
    }

}
}