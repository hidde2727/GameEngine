#include "network/WebsocketConnection.h"

#include "network/WebsocketHandler.h"
#include "network/WebHandler.h"

namespace Engine {
namespace Network {
    
    WebsocketConnection::WebsocketConnection(std::weak_ptr<WebHandler> webhandler, asio::ip::tcp::socket&& socket, Util::WeirdPointer<WebsocketHandler> handler) :
    _webhandler(webhandler),
    _socket(std::move(socket)),
    _receivingFrame(std::make_shared<Websocket::Frame>()),
    _timeout(webhandler.lock()->_context),
    _websocketHandler(handler)
    {
    }
    WebsocketConnection::~WebsocketConnection() {
        if(!_isStopped) Stop();
    }
    void WebsocketConnection::Start(const size_t uuid) {
        _uuid = uuid;
        ReceiveFirstPartHeader();
    }
    void WebsocketConnection::Stop() {
        if(ENGINE_NETWORK_VERBOSE_WEBSOCKET) LOG("[Network::WebsocketConnection] Stopping connection (" + std::to_string(_uuid) + ")")
        if(_isStopped || _closeAfterRead) return;
        _closeAfterRead = true;
        std::shared_ptr<Websocket::Frame> stopFrame = std::make_shared<Websocket::Frame>();
        stopFrame->SetClosingHandshake();
    }
    void WebsocketConnection::StopWithoutHandshake() {
        if(ENGINE_NETWORK_VERBOSE_WEBSOCKET) LOG("[Network::WebsocketConnection] Stopping connection (" + std::to_string(_uuid) + ") without handshake")
        if(_isStopped) return;
        _isStopped = true;
        _timeout.cancel();
        if(_socket.is_open()) _socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        _webhandler.lock()->StopWebsocketConnection(_uuid);

        // Post work for the main thread
        std::shared_ptr<WebsocketConnection> self = shared_from_this();
        asio::post(_webhandler.lock()->_requestHandler, [this, self]() {
            _websocketHandler->OnWebsocketStopInternal(self.get());
        });
    }
    void WebsocketConnection::ReceiveFirstPartHeader() {
        std::shared_ptr<WebsocketConnection> self = shared_from_this();

        asio::async_read(_socket, _receivingFrame->GetHeaderBuffer(), [this, self](const std::error_code& ec, size_t bytesTransfered) {
            if (ec) {
                if(ec==asio::error::eof) return StopWithoutHandshake();
				WARNING("[Network::WebsocketConnection] Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
			}
            if(ENGINE_NETWORK_VERBOSE_WEBSOCKET) LOG("[Network::WebsocketConnection] Received first part of header on connection (" + std::to_string(_uuid) + ")")
            _receivingFrame->OnHeaderRead();
            ReceiveSecondPartHeader();
        });
    }
    void WebsocketConnection::ReceiveSecondPartHeader() {
        std::shared_ptr<WebsocketConnection> self = shared_from_this();

        asio::async_read(_socket, _receivingFrame->GetSecondHeaderBuffer(), [this, self](const std::error_code& ec, size_t bytesTransfered) {
            if (ec) {
                if(ec==asio::error::eof) return StopWithoutHandshake();
				WARNING("[Network::WebsocketConnection] Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
			}
            if(ENGINE_NETWORK_VERBOSE_WEBSOCKET) LOG("[Network::WebsocketConnection] Received second part of header on connection (" + std::to_string(_uuid) + ")")
            _receivingFrame->OnSecondHeaderRead();
            ReceiveBody();
        });
    }
    void WebsocketConnection::ReceiveBody() {
        std::shared_ptr<WebsocketConnection> self = shared_from_this();

        asio::async_read(_socket, _receivingFrame->GetBodyBuffer(), [this, self](const std::error_code& ec, size_t bytesTransfered) {
            if (ec) {
                if(ec==asio::error::eof) return StopWithoutHandshake();
				WARNING("[Network::WebsocketConnection] Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
			}
            if(ENGINE_NETWORK_VERBOSE_WEBSOCKET) LOG("[Network::WebsocketConnection] Received body on connection (" + std::to_string(_uuid) + ")")
            if(_closeAfterRead) return StopWithoutHandshake();
            _receivingFrame->OnBodyRead();
            _closeAfterWrite = _receivingFrame->ShouldCloseAfterReturningMessage();
            _closeAfterRead = _receivingFrame->ShouldCloseAfterNextReceive();
            HandleReceive();
            if(!_closeAfterWrite) ReceiveFirstPartHeader();
        });

    }
    void WebsocketConnection::HandleReceive() {
        std::shared_ptr<WebsocketConnection> self = shared_from_this();

        if(_receivingFrame->HasMandatoryResponse()) {
            _receivingFrame->SetMandatoryResponse();
            _writeQueue.push(_receivingFrame);
            _receivingFrame = std::make_shared<Websocket::Frame>();
            // Set a timeout if we need to close
            if(_closeAfterWrite || _closeAfterRead) {            
                _timeout.expires_after(std::chrono::seconds(5));
                _timeout.async_wait([this, self](const std::error_code& e) {
                    if(e == asio::error::operation_aborted) return;
                    StopWithoutHandshake();
                    WARNING("[Network::WebsocketConnection] Connection closed by timeout and without the correct closing handshake")
                });
            }
            if(_writeQueue.size() == 1) Write();
            return;
        }
        std::shared_ptr<Websocket::Frame> frame = _receivingFrame;
        _receivingFrame = std::make_shared<Websocket::Frame>();
        // Post work for the main thread
        asio::post(_webhandler.lock()->_requestHandler, [this, self, frame]() {
            _websocketHandler->OnWebsocketMessageInternal(self.get(), *frame);
        });
    }

    void WebsocketConnection::SendData(std::shared_ptr<Websocket::Frame> data) {
        std::shared_ptr<WebsocketConnection> self = shared_from_this();

        // Post work for the networking thread
        asio::post(_webhandler.lock()->_context, [this, self, data]() {
            if(_closeAfterWrite) return;
            if(ENGINE_NETWORK_VERBOSE_WEBSOCKET) LOG("[Network::WebsocketConnection] Added frame to writing queue of connection (" + std::to_string(_uuid) + ")")
            _writeQueue.push(data);
            if(_writeQueue.size() == 1) Write();
        });
    }

    void WebsocketConnection::Write() {
		if (_writeQueue.empty()) return;

        std::shared_ptr<WebsocketConnection> self = shared_from_this();

        asio::async_write(_socket, _writeQueue.front()->GetWriteBuffers(),
        [this, self](const std::error_code& ec, std::size_t bytesTransferred)
        {
            if (ec) {
                if(ec==asio::error::eof) return StopWithoutHandshake();
				WARNING("[Network::WebsocketConnection] Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
            }
            if(ENGINE_NETWORK_VERBOSE_WEBSOCKET) LOG("[Network::WebsocketConnection] Successfully written message on connection (" + std::to_string(_uuid) + ")")
            _writeQueue.pop();
            if(_writeQueue.empty() && _closeAfterWrite) return StopWithoutHandshake();

            if (!_writeQueue.empty()) Write();
        });
    }

}
}