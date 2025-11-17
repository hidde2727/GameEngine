#ifndef ENGINE_NETWORK_WEBSOCKETCONNECTION_H
#define ENGINE_NETWORK_WEBSOCKETCONNECTION_H

#include "core/PCH.h"
#include "network/websocket/Frame.h"
#include "network/WebsocketHandler.h"
#include "util/WeirdPointer.h"

namespace Engine {
namespace Network {
    
    class WebHandler;
    class WebsocketConnection : public std::enable_shared_from_this<WebsocketConnection> {
    public:

        WebsocketConnection(std::weak_ptr<WebHandler> webhandler, asio::ip::tcp::socket&& socket, Util::WeirdPointer<WebsocketHandler> handler);
        ~WebsocketConnection();

        void Start(const size_t uuid);
        void Stop();

        /// Safe to call from the main thread
        void SendData(std::shared_ptr<Websocket::Frame> data);
        /// Safe to call from the main thread
        size_t GetUUID() { return _uuid; }
        Util::WeirdPointer<WebsocketHandler> GetHandler() { return _websocketHandler; }

    private:
        void ReceiveFirstPartHeader();
        void ReceiveSecondPartHeader();
        void ReceiveBody();
        void HandleReceive();
        void Write();
        void StopWithoutHandshake();

        std::weak_ptr<WebHandler> _webhandler;
        Util::WeirdPointer<WebsocketHandler> _websocketHandler;
        asio::ip::tcp::socket _socket;
        asio::system_timer _timeout;// The timeout is used to make sure we close after a closing frame
        size_t _uuid;

        asio::streambuf _inputBuffer;
        
        bool _isStopped = false;

        std::shared_ptr<Websocket::Frame> _receivingFrame;
        bool _closeAfterWrite = false;
        bool _closeAfterRead = false;
        std::queue<std::shared_ptr<Websocket::Frame>> _writeQueue;
    };

}
}

#endif