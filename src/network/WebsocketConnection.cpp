#include "network/WebHandler.h"

namespace Engine {
namespace Network {
    
    WebHandler::WebsocketConnection::WebsocketConnection(WebHandler* webhandler, asio::ip::tcp::socket&& socket) :
    _webhandler(webhandler),
    _socket(std::move(socket)),
    _receivingFrame(std::make_shared<Websocket::Frame>()),
    _timeout(webhandler->_context)
    {
    }
    WebHandler::WebsocketConnection::~WebsocketConnection() {
        if(!_isStopped) Stop();
    }
    void WebHandler::WebsocketConnection::Start(const size_t uuid) {
        _uuid = uuid;
        ReceiveFirstPartHeader();
    }
    void WebHandler::WebsocketConnection::Stop() {
        if(_isStopped || _closeAfterRead) return;
        _closeAfterRead = true;
        std::shared_ptr<Websocket::Frame> stopFrame = std::make_shared<Websocket::Frame>();
        stopFrame->SetClosingHandshake();
    }
    void WebHandler::WebsocketConnection::StopWithoutHandshake() {
        if(_isStopped) return;
        _isStopped = true;
        _timeout.cancel();
        if(_socket.is_open()) _socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        _webhandler->StopWebsocketConnection(_uuid);
    }
    void WebHandler::WebsocketConnection::ReceiveFirstPartHeader() {
        std::shared_ptr<WebHandler::WebsocketConnection> self = shared_from_this();

        asio::async_read(_socket, _receivingFrame->GetHeaderBuffer(), [this, self](const std::error_code& ec, size_t bytesTransfered) {
            if (ec) {
                if(ec==asio::error::eof) return StopWithoutHandshake();
				WARNING("Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
			}
            _receivingFrame->OnHeaderRead();
            ReceiveSecondPartHeader();
        });
    }
    void WebHandler::WebsocketConnection::ReceiveSecondPartHeader() {
        std::shared_ptr<WebHandler::WebsocketConnection> self = shared_from_this();

        asio::async_read(_socket, _receivingFrame->GetSecondHeaderBuffer(), [this, self](const std::error_code& ec, size_t bytesTransfered) {
            if (ec) {
                if(ec==asio::error::eof) return StopWithoutHandshake();
				WARNING("Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
			}
            _receivingFrame->OnSecondHeaderRead();
            ReceiveBody();
        });
    }
    void WebHandler::WebsocketConnection::ReceiveBody() {
        std::shared_ptr<WebHandler::WebsocketConnection> self = shared_from_this();

        asio::async_read(_socket, _receivingFrame->GetBodyBuffer(), [this, self](const std::error_code& ec, size_t bytesTransfered) {
            if (ec) {
                if(ec==asio::error::eof) return StopWithoutHandshake();
				WARNING("Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
			}
            if(_closeAfterRead) return StopWithoutHandshake();
            _receivingFrame->OnBodyRead();
            _closeAfterWrite = _receivingFrame->ShouldCloseAfterReturningMessage();
            _closeAfterRead = _receivingFrame->ShouldCloseAfterNextReceive();
            HandleReceive();
            if(!_closeAfterWrite) ReceiveFirstPartHeader();
        });

    }
    void WebHandler::WebsocketConnection::HandleReceive() {
        std::shared_ptr<WebHandler::WebsocketConnection> self = shared_from_this();

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
                    WARNING("Websocket connection closed by timeout and not by the correct closing handshake")
                });
            }
            if(_writeQueue.size() == 1) Write();
            return;
        }
        std::shared_ptr<Websocket::Frame> frame = _receivingFrame;
        _receivingFrame = std::make_shared<Websocket::Frame>();
        // Post work for the main thread
        asio::post(_webhandler->_requestHandler, [this, self, frame]() {
            _webhandler->_websocketHandler(*frame, *this);
        });
    }

    void WebHandler::WebsocketConnection::SendData(std::shared_ptr<Websocket::Frame> data) {
        std::shared_ptr<WebHandler::WebsocketConnection> self = shared_from_this();

        // Post work for the networking thread
        asio::post(_webhandler->_context, [this, self, data]() {
            if(_closeAfterWrite) return;
            _writeQueue.push(data);
            if(_writeQueue.size() == 1) Write();
        });
    }

    void WebHandler::WebsocketConnection::Write() {
		if (_writeQueue.empty()) return;

        std::shared_ptr<WebHandler::WebsocketConnection> self = shared_from_this();

        asio::async_write(_socket, _writeQueue.front()->GetWriteBuffers(),
        [this, self](const std::error_code& ec, std::size_t bytesTransferred)
        {
            if (ec) {
                if(ec==asio::error::eof) return StopWithoutHandshake();
				WARNING("Connection: " + std::to_string(_uuid) + " stopped because: " + ec.message())
				Stop();
				return;
            }
            _writeQueue.pop();
            if(_writeQueue.empty() && _closeAfterWrite) return StopWithoutHandshake();

            if (!_writeQueue.empty()) Write();
        });
    }

}
}