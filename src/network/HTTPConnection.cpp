#include "network/WebHandler.h"

#include "util/Hashing.h"

namespace Engine {
namespace Network {

    WebHandler::HTTPConnection::HTTPConnection(WebHandler* webhandler, asio::ip::tcp::socket&& socket) :
    _webhandler(webhandler),
    _socket(std::move(socket)),
    _timeout(webhandler->_context)
    {
    }
    WebHandler::HTTPConnection::~HTTPConnection() {
        if(!_isStopped) Stop();
    }
    
    void WebHandler::HTTPConnection::Start(const size_t uuid) {
        _uuid = uuid;
        ScheduleTimeoutCheck();
        ReceiveHeader();
    }
    void WebHandler::HTTPConnection::Stop() {
        if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Stopping connection (" + std::to_string(_uuid) + ")")
        if(_isStopped) return;
        _isStopped = true;
        if(_socket.is_open()) _socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        _timeout.cancel();
        _webhandler->StopHTTPConnection(_uuid);
        if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Stopped connection (" + std::to_string(_uuid) + ")")
    }
    void WebHandler::HTTPConnection::ReceiveHeader() {
        ScheduleTimeoutCheck();
        std::shared_ptr<WebHandler::HTTPConnection> self = shared_from_this();

        asio::async_read_until(_socket, _inputBuffer, "\r\n\r\n",
        [this, self](const std::error_code& ec, std::size_t bytesTransferred)
        {
            if (ec) {
				if(ec!=asio::error::eof) WARNING("Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
			}
            
            std::istream is(&_inputBuffer);
            _header = std::make_shared<HTTP::RequestHeader>();
            _header->Parse(is, bytesTransferred);
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Connection (" + std::to_string(_uuid) + ") received header: " + _header->GetHead())

            if(_header->GetHeader("Connection").find("Upgrade")!=std::string::npos && _header->GetHeader("Upgrade").find("websocket")!=std::string::npos) {
                if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Upgrading connection (" + std::to_string(_uuid) + ")")
                UpgradeConnection();
                return;
            }

            std::string contentLength = _header->GetHeader("Content-Length");
            if(contentLength.size()>0) {
                ReceiveBody(stoi(contentLength));
                return;
            }
            // There is no body, so create an empty one
            _body = std::make_shared<std::vector<uint8_t>>();
            HandleRequest();
            ReceiveHeader();
        }
        );
    }
    void WebHandler::HTTPConnection::ReceiveBody(const size_t size) {
        ScheduleTimeoutCheck();
        std::shared_ptr<WebHandler::HTTPConnection> self = shared_from_this();

        _body = std::make_shared<std::vector<uint8_t>>();
        _body->resize(size);

        asio::async_read(_socket, asio::buffer(_body->data(), _body->size()),
        [this, self](const std::error_code& ec, std::size_t bytesTransferred)
        {
            if (ec) {
				if(ec!=asio::error::eof) WARNING("[HTTPConnection] Connection (" + std::to_string(_uuid) + ") stopped because: " + ec.message())
				Stop();
				return;
			}
            
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Received body on connection (" + std::to_string(_uuid) + ") with head (" + _header->GetHead() + ")")

            HandleRequest();
            ReceiveHeader();
        }
        );
    }
    void WebHandler::HTTPConnection::Write() {
		if (_writeQueue.empty()) return;
        ScheduleTimeoutCheck();

        std::shared_ptr<WebHandler::HTTPConnection> self = shared_from_this();

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

    void WebHandler::HTTPConnection::HandleRequest() {
        std::shared_ptr<WebHandler::HTTPConnection> self = shared_from_this();

        std::shared_ptr<HTTP::RequestHeader> header = _header;
        std::shared_ptr<std::vector<uint8_t>> body = _body;
        // Post work for the main thread
        asio::post(asio::bind_executor(_webhandler->_requestHandler, 
        [this, self, header, body]() {
            std::shared_ptr<HTTP::Response> response = std::make_shared<HTTP::Response>(_webhandler->_fileManager);
            // Default response initializing
            response->SetResponseCode(HTTP::ResponseCode::OK);
            if(header->GetHeader("Connection").find("close")!=std::string::npos) {
                _connectionKeepAlive = true;
                response->SetHeader("Connection", "keep-alive");
                response->SetHeader("Keep-Alive", "timeout=5");
            }

            _webhandler->_httpHandler(*header, *body, *response);
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Response for connection (" + std::to_string(_uuid) + ") with request (" + header->GetHead() + ") created")

            // Post work for the networking thread
            asio::post(_webhandler->_context, [this, self, response]() {
                _writeQueue.push(response);
                if(_writeQueue.size() == 1) Write();
            });
        }));
    }

    void WebHandler::HTTPConnection::UpgradeConnection() {
        _timeout.cancel();
        std::shared_ptr<WebHandler::HTTPConnection> self = shared_from_this();
        std::shared_ptr<HTTP::RequestHeader> header = _header;

        // Post work for the main thread
        asio::post(_webhandler->_requestHandler, [this, self, header]() {
            std::shared_ptr<HTTP::Response> response = std::make_shared<HTTP::Response>(_webhandler->_fileManager);
            const bool allowUpgrade = _webhandler->_upgradeHandler(*header);
            // Default response initializing
            if(allowUpgrade) {
                response->SetResponseCode(HTTP::ResponseCode::SwitchingProtocols);
                response->SetHeader("Connection", "Upgrade");
                response->SetHeader("Upgrade", "websocket");
                response->SetHeader("Sec-WebSocket-Accept",
                    Util::Base64Encode(Util::SHA1(header->GetHeader("Sec-WebSocket-Key") + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))
                );
                _connectionKeepAlive = true;
                if(ENGINE_NETWORK_VERBOSE_HTTP_WEBSOCKET) LOG("[HTTPConnection] Allowed to upgrade connection (" + std::to_string(_uuid) + ")")
            } else {
                response->SetResponseCode(HTTP::ResponseCode::Forbidden);
                if(ENGINE_NETWORK_VERBOSE_HTTP_WEBSOCKET) LOG("[HTTPConnection] Disallowed to upgrade connection (" + std::to_string(_uuid) + ")")
            }

            // Post work for the networking thread
            asio::post(_webhandler->_context, [this, self, response, header, allowUpgrade]() {
                _writeQueue.push(response);
                if(_writeQueue.size() == 1) Write();
                if(!allowUpgrade) return;
                _webhandler->UpgradeHTTPConnection(_uuid, std::move(_socket), *header);
            });
        });
    }

    void WebHandler::HTTPConnection::ScheduleTimeoutCheck() {
        return;
        std::shared_ptr<WebHandler::HTTPConnection> self = shared_from_this();
        
        _timeout.expires_after(std::chrono::seconds(5));
        _timeout.async_wait([this, self](const std::error_code& e) {
            if(e == asio::error::operation_aborted) return;
            if(ENGINE_NETWORK_VERBOSE_HTTP) LOG("[HTTPConnection] Stopping connection (" + std::to_string(_uuid) + ") because of timeout")
            Stop();
        });
    }

}
}