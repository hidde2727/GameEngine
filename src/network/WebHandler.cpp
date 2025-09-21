#include "network/WebHandler.h"

namespace Engine {
namespace Network {
    
    WebHandler::WebHandler() :  _acceptor(_context) {
    }

    void WebHandler::Start(
        const Util::FileManager* fileManager,
        ENGINE_NETWORK_HTTPHANDLER_FUNCTION httpHandler, 
        ENGINE_NETWORK_UPGRADEHANDLER_FUNCTION upgradeHandler, 
        ENGINE_NETWORK_WEBSOCKETHANDLER_FUNCTION websocketHandler,
        ENGINE_NETWORK_ONWEBSOCKETSTART_FUNCTION onWebsocketStart,
        ENGINE_NETWORK_ONWEBSOCKETSTOP_FUNCTION onWebsocketStop
    ) {
        _httpHandler = httpHandler;
        _upgradeHandler = upgradeHandler;
        _websocketHandler = websocketHandler;
        _onWebsocketStart = onWebsocketStart;
        _onWebsocketStop = onWebsocketStop;

        Start(fileManager);
    }
    void WebHandler::Start(const Util::FileManager* fileManager) {
        _fileManager = fileManager;
        try {
            _localAddress = GetLocalAdress();
            LOG("[WebHandler] Opening for web requests on 'http://" + _localAddress + ":8000'")
        } catch(std::exception exc) {
            WARNING("Failed to get the local adress, are you connected to the internet?")
            return;
        }
        try {
            asio::ip::tcp::resolver resolver(_context);
            asio::ip::tcp::endpoint endpoint = *resolver.resolve(_localAddress, "8000").begin();
            _acceptor.open(endpoint.protocol());
            _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
            _acceptor.bind(endpoint);
            _acceptor.listen();

            AwaitConnection();
            _thread = std::thread([&] { try { _context.run(); } catch (std::exception err) { THROW("Webhandler experienced an error:\n" + std::string(err.what())); return 1;} return 0; });
        } catch(asio::system_error err) {
            WARNING("Failed to open an acceptor to the adress '" + _localAddress + "'. Because: '" + err.code().message() + "'")
        }
    }
    void WebHandler::Stop() {
        _context.stop();
        if(_thread.joinable()) _thread.join();
    }
    
    void WebHandler::HandleRequests() {
        auto count = _requestHandler.poll();
        if(_requestHandler.stopped()) _requestHandler.restart();
    }

    void WebHandler::AwaitConnection() {
		_acceptor.async_accept(
            asio::make_strand(_context),
            [this](const std::error_code& ec, asio::ip::tcp::socket socket)
            {
                if (!_acceptor.is_open()) return;
                if (!ec) {
                    std::shared_ptr<HTTPConnection>newConnection = std::make_shared<HTTPConnection>(this, std::move(socket));
                    size_t uuid = reinterpret_cast<size_t>(newConnection.get());// Using the memory adress as the uuid (lifetime of the uuid and the unique_ptr are the same, thus the uuid is unique)
                    newConnection->Start(uuid);
                    _httpConnections[uuid] = std::move(newConnection);
                }
                AwaitConnection();
            }
        );
        asio::detail::event event;
    }
    
    void WebHandler::StopHTTPConnection(const size_t uuid) {
        _httpConnections.erase(uuid);
    }
    void WebHandler::StopWebsocketConnection(const size_t uuid) {
        // Post work for the main thread
        std::shared_ptr<WebsocketConnection> connection = _websocketConnections[uuid];
        asio::post(_requestHandler, [this, connection, uuid]() {
            _onWebsocketStop(*connection, uuid);
        });
        _websocketConnections.erase(uuid);
    }
    void WebHandler::UpgradeHTTPConnection(const size_t olduuid, asio::ip::tcp::socket&& socket, HTTP::RequestHeader& requestHeader) {
        _httpConnections.erase(olduuid);
        std::shared_ptr<WebsocketConnection>newConnection = std::make_shared<WebsocketConnection>(this, std::move(socket));
        size_t uuid = reinterpret_cast<size_t>(newConnection.get());// Using the memory adress as the uuid (lifetime of the uuid and the unique_ptr are the same, thus the uuid is unique)
        newConnection->Start(uuid);
        _websocketConnections[uuid] = std::move(newConnection);
        // Post work for the main thread
        asio::post(_requestHandler, [this, &newConnection, uuid, &requestHeader]() {
            _onWebsocketStart(*newConnection, uuid, requestHeader);
        });
    }

    std::string WebHandler::GetLocalAdress() {
        asio::ip::tcp::resolver resolver(_context);
        const auto query = resolver.resolve(asio::ip::host_name(), "");
        asio::ip::basic_resolver_iterator<asio::ip::tcp> iter = query.begin();
        asio::ip::basic_resolver_iterator<asio::ip::tcp> end = query.end();
        while (iter != end)
        {
            if (iter->endpoint().address().is_v4()
            && !iter->endpoint().address().is_loopback()
            && !iter->endpoint().address().is_multicast()
            && !iter->endpoint().address().is_unspecified())
            {
            return iter->endpoint().address().to_string();
            }

            ++iter;
        }
        return 0;
    }

}
}